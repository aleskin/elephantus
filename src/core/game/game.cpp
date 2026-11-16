#include "core/game/game.hpp"

#include <stdexcept>
#include <utility>

namespace ce::core
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

Game::Game(Campaign campaign, LevelLoader loader, Tunables tunables)
    : campaign_(std::move(campaign))
    , loader_(std::move(loader))
    , tunables_(tunables)
    , world_(first_level(campaign_, loader_), tunables_)
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

std::vector<Event> Game::step(const InputFrame& input, double dt)
{
    if (phase_ == GamePhase::LevelComplete && input.pressed(GameButton::A))
    {
        // TODO(CEL-00012): what happens after a level is Polina's call; for now A moves on.
        next_level();
        return {};
    }
    if (phase_ == GamePhase::LevelFailed)
    {
        failed_for_ += dt;
        if (failed_for_ >= tunables_.restart_delay)
        {
            world_ = World{world_.level(), tunables_};
            phase_ = GamePhase::Playing;
        }
        return {};
    }
    if (phase_ != GamePhase::Playing)
    {
        return {};
    }
    std::vector<Event> events = world_.step(input, dt);
    for (const Event& event : events)
    {
        if (event.kind == EventKind::LevelFinished)
        {
            phase_ = GamePhase::LevelComplete;
        }
        if (event.kind == EventKind::Fell || event.kind == EventKind::HoopTouched)
        {
            phase_ = GamePhase::LevelFailed;
            failed_for_ = 0.0;
        }
    }
    return events;
}

void Game::next_level()
{
    if (phase_ != GamePhase::LevelComplete)
    {
        return;
    }
    if (level_index_ + 1 >= campaign_.levels.size())
    {
        phase_ = GamePhase::CampaignComplete;
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

} // namespace ce::core
