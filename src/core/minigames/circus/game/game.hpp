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
    LevelFailed,      // a fall: minus an attempt, a pause, then the same level again
    LevelComplete,    // on the podium: A goes to the next level
    CampaignComplete, // the last level is finished: the points are shown, A ends the mini-game
    GameOver,         // no attempts left: the points are shown, A ends the mini-game
    Done,
};

// The game from the first level to the last (owner, 2026-11-27, 2026-11-28): points for
// every hoop add up over the levels; a fall takes an attempt and the same level starts
// over; the podium leads to the next level; no attempts left or the last level done -
// the points are the result.
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

    // Points of this run so far.
    int score() const;
    // 1 for the first attempt; every fall starts the next one.
    int attempt() const;
    int attempt_count() const;
    // Attempts after this one.
    int attempts_left() const;
    // The points kept at the last podium: the result of this run.
    int best() const;
    // The best points ever: the records file or this run, whichever is more.
    int record() const;

    std::vector<Event> step(const InputFrame& input, double dt);
    // From LevelComplete: the next level.
    void next_level();
    // New numbers from the game config, used right away and after every restart.
    void set_tunables(const Tunables& tunables);

private:
    void restart_level();
    void keep_score();

    Campaign campaign_;
    LevelLoader loader_;
    std::size_t level_index_ = 0;
    Tunables tunables_;
    World world_;
    GamePhase phase_ = GamePhase::Playing;
    double failed_for_ = 0.0;
    int score_ = 0;
    int level_start_score_ = 0;
    int attempt_count_;
    int attempt_ = 1;
    int best_ = 0;
    int record_;
};

} // namespace ce::core::circus
