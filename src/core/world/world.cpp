#include "core/world/world.hpp"

#include <utility>

namespace ce::core
{

World::World(LevelData level)
    : level_(std::move(level))
{
    elephant_.x = level_.start_x;
}

const LevelData& World::level() const
{
    return level_;
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

std::vector<Event> World::step(const InputFrame& input, double dt)
{
    static_cast<void>(input);
    std::vector<Event> events;
    if (finished_)
    {
        return events;
    }
    time_ += dt;
    if (elephant_.x >= level_.length)
    {
        finished_ = true;
        events.push_back(Event{EventKind::LevelFinished});
    }
    return events;
}

} // namespace ce::core
