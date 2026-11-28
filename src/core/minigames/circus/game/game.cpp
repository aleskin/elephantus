#include "core/minigames/circus/game/game.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace ce::core::circus
{

namespace
{

LevelData first_level(const Campaign& campaign, const Game::LevelLoader& loader)
{
    if (campaign.levels.empty())
    {
        throw std::invalid_argument("campaign has no levels");
    }
    return loader(campaign.levels.front());
}

} // namespace

Game::Game(Campaign campaign, LevelLoader loader, Tunables tunables, int record)
    : campaign_(std::move(campaign))
    , loader_(std::move(loader))
    , tunables_(tunables)
    , world_(first_level(campaign_, loader_), tunables_)
    , attempt_count_(tunables_.attempt_count())
    , record_(record)
{
}

GamePhase Game::phase() const
{
    return phase_;
}

std::size_t Game::level_index() const
{
    return level_index_;
}

std::size_t Game::level_count() const
{
    return campaign_.levels.size();
}

const World& Game::world() const
{
    return world_;
}

int Game::score() const
{
    return score_;
}

int Game::attempt() const
{
    return attempt_;
}

int Game::attempt_count() const
{
    return attempt_count_;
}

int Game::attempts_left() const
{
    return attempt_count_ - attempt_;
}

int Game::best() const
{
    return best_;
}

int Game::record() const
{
    return std::max(record_, best_);
}

std::vector<Event> Game::step(const InputFrame& input, double dt)
{
    const bool a = input.pressed(GameButton::A);
    switch (phase_)
    {
    case GamePhase::Playing:
        break;
    case GamePhase::LevelFailed:
        failed_for_ += dt;
        if (failed_for_ >= tunables_.restart_delay)
        {
            if (attempts_left() > 0)
            {
                ++attempt_;
                start_attempt();
            }
            else
            {
                phase_ = GamePhase::GameOver;
            }
        }
        return {};
    case GamePhase::LevelComplete:
        if (a)
        {
            // TODO(CEL-00012): what happens after a level is Polina's call; for now A moves on.
            next_level();
        }
        return {};
    case GamePhase::CampaignComplete:
        if (a)
        {
            ++attempt_;
            start_attempt();
        }
        return {};
    case GamePhase::GameOver:
        if (a)
        {
            phase_ = GamePhase::Done;
        }
        return {};
    case GamePhase::Done:
        return {};
    }

    std::vector<Event> events = world_.step(input, dt);
    for (const Event& event : events)
    {
        if (event.kind == EventKind::HoopPassed)
        {
            score_ += tunables_.points_per_hoop();
        }
        if (event.kind == EventKind::LevelFinished)
        {
            if (level_index_ + 1 < campaign_.levels.size())
            {
                phase_ = GamePhase::LevelComplete;
            }
            else
            {
                end_attempt();
                phase_ = attempts_left() > 0 ? GamePhase::CampaignComplete : GamePhase::GameOver;
            }
        }
    }
    if (world_.failed())
    {
        end_attempt();
        phase_ = GamePhase::LevelFailed;
        failed_for_ = 0.0;
    }
    return events;
}

void Game::next_level()
{
    if (phase_ != GamePhase::LevelComplete || level_index_ + 1 >= campaign_.levels.size())
    {
        return;
    }
    ++level_index_;
    world_ = World{loader_(campaign_.levels[level_index_]), tunables_};
    phase_ = GamePhase::Playing;
}

void Game::set_tunables(const Tunables& tunables)
{
    tunables_ = tunables;
    world_.set_tunables(tunables_);
}

void Game::start_attempt()
{
    score_ = 0;
    if (level_index_ != 0)
    {
        level_index_ = 0;
        world_ = World{loader_(campaign_.levels.front()), tunables_};
    }
    else
    {
        world_ = World{world_.level(), tunables_};
    }
    phase_ = GamePhase::Playing;
}

void Game::end_attempt()
{
    best_ = std::max(best_, score_);
}

} // namespace ce::core::circus
