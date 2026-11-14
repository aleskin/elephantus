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

Game::Game(Campaign campaign, LevelLoader loader)
    : campaign_(std::move(campaign))
    , loader_(std::move(loader))
    , world_(first_level(campaign_, loader_))
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
    world_ = World{loader_(campaign_.levels[level_index_])};
    phase_ = GamePhase::Playing;
}

} // namespace ce::core
