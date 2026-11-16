#pragma once

#include "core/minigames/circus/game/game.hpp"
#include "core/party/minigame.hpp"

#include <filesystem>

namespace ce::core::circus
{

// The circus elephant, the first mini-game of Elephantus. One player per run.
extern const MinigameInfo info;

// assets/minigames/circus/levels/campaign.txt
std::filesystem::path campaign_path(const std::filesystem::path& assets);

class CircusMinigame : public Minigame
{
public:
    // Throws std::invalid_argument when the setup does not fit the circus (one player).
    CircusMinigame(Campaign campaign, Game::LevelLoader loader, const KvDocument& config,
                   MinigameSetup setup = {});

    const Game& game() const;

    const MinigameInfo& info() const override;
    void step(std::span<const InputFrame> players, double dt) override;
    MinigameOutcome outcome() const override;
    int points(int player) const override;
    void apply_config(const KvDocument& config) override;

private:
    Game game_;
};

} // namespace ce::core::circus
