#pragma once

#include "core/level/level.hpp"
#include "core/world/world.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <vector>

namespace ce::core
{

enum class GamePhase
{
    Playing,
    LevelFailed, // the elephant fell or touched a hoop; the level starts over soon
    LevelComplete,
    CampaignComplete,
};

// The game from the first level to the last: plays a level, starts it over after a
// fall, waits after it is finished, moves on to the next one.
class Game
{
public:
    using LevelLoader = std::function<LevelData(const std::filesystem::path&)>;

    explicit Game(Campaign campaign, LevelLoader loader = load_level, Tunables tunables = {});

    GamePhase phase() const;
    std::size_t level_index() const;
    std::size_t level_count() const;
    const World& world() const;

    std::vector<Event> step(const InputFrame& input, double dt);
    // From LevelComplete: the next level, or CampaignComplete after the last one.
    void next_level();

private:
    Campaign campaign_;
    LevelLoader loader_;
    std::size_t level_index_ = 0;
    Tunables tunables_;
    World world_;
    GamePhase phase_ = GamePhase::Playing;
    double failed_for_ = 0.0;
};

} // namespace ce::core
