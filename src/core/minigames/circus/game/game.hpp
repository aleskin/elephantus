#pragma once

#include "core/minigames/circus/level/level.hpp"
#include "core/minigames/circus/world/world.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <vector>

namespace ce::core::circus
{

enum class GamePhase
{
    Playing,
    LevelFailed, // the elephant fell or touched a hoop: the attempt is over, a pause
    LevelComplete,
    CampaignComplete, // the last level is finished: the attempt is over, A starts the next one
    GameOver,         // no attempts left: the points are shown, A ends the mini-game
    Done,
};

// The game from the first level to the last, in attempts (owner, 2026-11-27): points for
// every hoop, a fall or a touched hoop ends the attempt, so does the last level; the best
// attempt is the result. The next attempt starts from the first level with no points.
// TODO(CEL-00056): whether the points of a fallen attempt count and whether finishing
// takes an attempt - temporary rules until the owner answers.
class Game
{
public:
    using LevelLoader = std::function<LevelData(const std::filesystem::path&)>;

    // record: the best points ever, from the records file; only shown.
    explicit Game(Campaign campaign, LevelLoader loader = load_level, Tunables tunables = {},
                  int record = 0);

    GamePhase phase() const;
    std::size_t level_index() const;
    std::size_t level_count() const;
    const World& world() const;

    // Points of the attempt being played (or just over).
    int score() const;
    // 1 for the first attempt.
    int attempt() const;
    int attempt_count() const;
    // Attempts after this one.
    int attempts_left() const;
    // The best finished attempt of this run.
    int best() const;
    // The best points ever: the records file or this run, whichever is more.
    int record() const;

    std::vector<Event> step(const InputFrame& input, double dt);
    // From LevelComplete: the next level.
    void next_level();
    // New numbers from the game config, used right away and after every restart.
    void set_tunables(const Tunables& tunables);

private:
    void start_attempt();
    void end_attempt();

    Campaign campaign_;
    LevelLoader loader_;
    std::size_t level_index_ = 0;
    Tunables tunables_;
    World world_;
    GamePhase phase_ = GamePhase::Playing;
    double failed_for_ = 0.0;
    int score_ = 0;
    int attempt_count_;
    int attempt_ = 1;
    int best_ = 0;
    int record_;
};

} // namespace ce::core::circus
