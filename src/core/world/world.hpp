#pragma once

#include "core/elephant/elephant.hpp"
#include "core/input/combo.hpp"
#include "core/input/input_frame.hpp"
#include "core/level/level.hpp"
#include "core/objects/hoop.hpp"
#include "core/settings/tunables.hpp"

#include <cstddef>
#include <vector>

namespace ce::core
{

// What happened during a step. Sound, interface and the game flow react to events;
// the world itself knows nothing about them.
enum class EventKind
{
    Jumped,
    Landed,
    HoopPassed,
    HoopTouched, // the level starts over
    Fell,        // back on four legs: the level starts over
    LevelFinished,
};

struct Event
{
    EventKind kind;
};

// Does a ball touch a hoop standing across the path? In the side view only the opening
// of the ring is free: everything from the floor up to the bottom of the ring counts as
// solid, so the elephant cannot walk under it, and so does the top of the ring.
// TODO(CEL-00030): through the ring for now; over it, if Polina says so.
bool touches_hoop(const Hoop& hoop, double x, double centre_y, double radius);

// One level being played: the elephant, the objects and the rules from SPEC.md.
class World
{
public:
    explicit World(LevelData level, Tunables tunables = {});

    const LevelData& level() const;
    const Tunables& tunables() const;
    const Elephant& elephant() const;
    double time() const;
    bool finished() const;
    bool failed() const;

    std::vector<Event> step(const InputFrame& input, double dt);

private:
    void walk(bool jump, bool a_held, double dt, std::vector<Event>& events);
    void fly(double dt, std::vector<Event>& events);
    void check_hoops(std::vector<Event>& events);
    void fall(EventKind why, std::vector<Event>& events);

    LevelData level_;
    Tunables tunables_;
    std::vector<Hoop> hoops_; // sorted by x
    std::size_t next_hoop_ = 0;
    Combo salto_;
    Elephant elephant_;
    double time_ = 0.0;
    bool finished_ = false;
};

} // namespace ce::core
