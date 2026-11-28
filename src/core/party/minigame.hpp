#pragma once

#include "core/input/input_frame.hpp"
#include "core/text/kv_file.hpp"

#include <span>
#include <string_view>

namespace ce::core
{

// Elephantus is a party game: a collection of mini-games for up to four players.
inline constexpr int max_players = 4;

// How a mini-game is played by several people (SPEC.md, "Рамка игры").
enum class PlayStyle
{
    Turns,    // one player per run; everybody has a go, then the points are compared
    Together, // two or more players at the same time
};

// Who takes part in one run of a mini-game.
struct MinigameSetup
{
    int players = 1;
    // The best points ever in this mini-game, from the records file; for the screen.
    int record = 0;
};

// Turns: exactly one player. Together: from two to max_players.
bool fits(PlayStyle style, MinigameSetup setup);

enum class MinigameOutcome
{
    Playing,
    Won,
    Lost,
};

// What the menu and the game config know about a mini-game.
struct MinigameInfo
{
    // Folder name (src/core/minigames/<id>/, assets/minigames/<id>/) and the prefix of its
    // sections in the game config: [<id>.something].
    std::string_view id;
    // Name in the menu.
    std::string_view title;
    PlayStyle style = PlayStyle::Turns;
    // Reads this mini-game's sections of the game config and throws KvError when they are
    // wrong, so a typo is shown before anything changes.
    void (*check_config)(const KvDocument& config) = nullptr;
};

// One run of a mini-game: the same steps for every mini-game, its own rules inside.
class Minigame
{
public:
    Minigame() = default;
    Minigame(const Minigame&) = delete;
    Minigame& operator=(const Minigame&) = delete;
    virtual ~Minigame() = default;

    virtual const MinigameInfo& info() const = 0;
    // One logic step. players[i] is the buttons of the i-th player of this run; there are
    // as many as MinigameSetup::players.
    virtual void step(std::span<const InputFrame> players, double dt) = 0;
    virtual MinigameOutcome outcome() const = 0;
    // Points of the i-th player of this run so far; a new record when above
    // MinigameSetup::record.
    virtual int points(int player) const = 0;
    // The game config was changed while the game runs; it has passed check_config.
    virtual void apply_config(const KvDocument& config) = 0;
};

} // namespace ce::core
