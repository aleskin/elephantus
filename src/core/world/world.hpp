#pragma once

#include "core/elephant/elephant.hpp"
#include "core/input/input_frame.hpp"
#include "core/level/level.hpp"

#include <vector>

namespace ce::core
{

// What happened during a step. Sound, interface and the game flow react to events;
// the world itself knows nothing about them.
enum class EventKind
{
    LevelFinished,
};

struct Event
{
    EventKind kind;
};

// One level being played: the elephant, the objects and the rules.
class World
{
public:
    explicit World(LevelData level);

    const LevelData& level() const;
    const Elephant& elephant() const;
    double time() const;
    bool finished() const;

    std::vector<Event> step(const InputFrame& input, double dt);

private:
    LevelData level_;
    Elephant elephant_;
    double time_ = 0.0;
    bool finished_ = false;
};

} // namespace ce::core
