#include "core/world/world.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <variant>

namespace ce::core
{

namespace
{

std::vector<Hoop> hoops_of(const LevelData& level)
{
    std::vector<Hoop> hoops;
    for (const LevelObject& object : level.objects)
    {
        if (const Hoop* hoop = std::get_if<Hoop>(&object))
        {
            hoops.push_back(*hoop);
        }
    }
    std::sort(hoops.begin(), hoops.end(),
              [](const Hoop& a, const Hoop& b)
              {
                  return a.x < b.x;
              });
    return hoops;
}

} // namespace

bool touches_hoop(const Hoop& hoop, double x, double centre_y, double radius)
{
    const double dx = x - hoop.x;
    if (std::abs(dx) >= radius)
    {
        return false;
    }
    const double half = std::sqrt(radius * radius - dx * dx);
    return centre_y - half <= hoop.bottom() || centre_y + half >= hoop.top();
}

World::World(LevelData level, Tunables tunables)
    : level_(std::move(level))
    , tunables_(tunables)
    , hoops_(hoops_of(level_))
    , salto_({GameButton::X, GameButton::B}, tunables_.salto_window)
{
    elephant_.x = level_.start_x;
}

const LevelData& World::level() const
{
    return level_;
}

const Tunables& World::tunables() const
{
    return tunables_;
}

const Elephant& World::elephant() const
{
    return elephant_;
}

double World::time() const
{
    return time_;
}

bool World::finished() const
{
    return finished_;
}

bool World::failed() const
{
    return elephant_.state == ElephantState::Fallen;
}

std::vector<Event> World::step(const InputFrame& input, double dt)
{
    std::vector<Event> events;
    if (finished_ || failed())
    {
        return events;
    }
    time_ += dt;
    const bool jump = salto_.step(input, dt);
    const bool a_held = input.held(GameButton::A);

    switch (elephant_.state)
    {
    case ElephantState::Standing:
        if (a_held)
        {
            elephant_.state = ElephantState::Walking;
        }
        break;
    case ElephantState::Walking:
        walk(jump, a_held, dt, events);
        break;
    case ElephantState::Jumping:
        fly(dt, events);
        break;
    case ElephantState::Fallen:
        break;
    }
    if (failed())
    {
        return events;
    }

    check_hoops(events);
    if (!failed() && elephant_.x >= level_.length)
    {
        finished_ = true;
        events.push_back(Event{EventKind::LevelFinished});
    }
    return events;
}

void World::walk(bool jump, bool a_held, double dt, std::vector<Event>& events)
{
    elephant_.x += tunables_.walk_speed * dt;
    if (jump)
    {
        elephant_.state = ElephantState::Jumping;
        elephant_.vy = tunables_.jump_speed();
        elephant_.air_time = 0.0;
        elephant_.salto = 0.0;
        elephant_.without_a = 0.0;
        events.push_back(Event{EventKind::Jumped});
        return;
    }
    if (a_held || salto_.pending())
    {
        elephant_.without_a = 0.0;
        return;
    }
    elephant_.without_a += dt;
    if (elephant_.without_a > tunables_.fall_grace)
    {
        fall(EventKind::Fell, events);
    }
}

void World::fly(double dt, std::vector<Event>& events)
{
    elephant_.x += tunables_.walk_speed * dt;
    elephant_.vy -= tunables_.gravity * dt;
    elephant_.y += elephant_.vy * dt;
    elephant_.air_time += dt;
    elephant_.salto = std::min(1.0, elephant_.air_time / tunables_.air_time());
    if (elephant_.y <= 0.0)
    {
        elephant_.y = 0.0;
        elephant_.vy = 0.0;
        elephant_.salto = 0.0;
        elephant_.state = ElephantState::Walking;
        events.push_back(Event{EventKind::Landed});
    }
}

void World::check_hoops(std::vector<Event>& events)
{
    while (next_hoop_ < hoops_.size())
    {
        const Hoop& hoop = hoops_[next_hoop_];
        if (touches_hoop(hoop, elephant_.x, elephant_.centre_y(), Elephant::body_radius))
        {
            fall(EventKind::HoopTouched, events);
            return;
        }
        if (elephant_.x - Elephant::body_radius < hoop.x)
        {
            return;
        }
        ++next_hoop_;
        events.push_back(Event{EventKind::HoopPassed});
    }
}

void World::fall(EventKind why, std::vector<Event>& events)
{
    elephant_.state = ElephantState::Fallen;
    elephant_.y = 0.0;
    elephant_.vy = 0.0;
    elephant_.salto = 0.0;
    events.push_back(Event{why});
}

} // namespace ce::core
