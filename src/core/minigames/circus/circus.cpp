#include "core/minigames/circus/circus.hpp"

#include "core/minigames/circus/settings/config.hpp"

#include <stdexcept>
#include <utility>

namespace ce::core::circus
{

const MinigameInfo info{"circus", "Цирковой слоник", PlayStyle::Turns, &check_config};

std::filesystem::path campaign_path(const std::filesystem::path& assets)
{
    return assets / "minigames" / std::filesystem::path{info.id} / "levels" / "campaign.txt";
}

namespace
{

MinigameSetup checked(MinigameSetup setup)
{
    if (!fits(info.style, setup))
    {
        throw std::invalid_argument("the circus is played by one player at a time");
    }
    return setup;
}

} // namespace

CircusMinigame::CircusMinigame(Campaign campaign, Game::LevelLoader loader,
                               const KvDocument& config, MinigameSetup setup)
    : game_((checked(setup), std::move(campaign)), std::move(loader), read_tunables(config))
{
}

const Game& CircusMinigame::game() const
{
    return game_;
}

std::vector<Event> CircusMinigame::take_events()
{
    return std::exchange(events_, {});
}

const MinigameInfo& CircusMinigame::info() const
{
    return circus::info;
}

void CircusMinigame::step(std::span<const InputFrame> players, double dt)
{
    const std::vector<Event> events =
        game_.step(players.empty() ? InputFrame{} : players.front(), dt);
    events_.insert(events_.end(), events.begin(), events.end());
}

MinigameOutcome CircusMinigame::outcome() const
{
    // A fall or a touched hoop starts the level over, so the circus is never lost.
    return game_.phase() == GamePhase::CampaignComplete ? MinigameOutcome::Won
                                                        : MinigameOutcome::Playing;
}

int CircusMinigame::points(int /*player*/) const
{
    // TODO(CEL-00040): points for a good salto and for the cup, once Polina decides.
    return 0;
}

void CircusMinigame::apply_config(const KvDocument& config)
{
    game_.set_tunables(read_tunables(config));
}

} // namespace ce::core::circus
