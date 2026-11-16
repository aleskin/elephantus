#include "core/party/minigame.hpp"

namespace ce::core
{

bool fits(PlayStyle style, MinigameSetup setup)
{
    if (style == PlayStyle::Turns)
    {
        return setup.players == 1;
    }
    return setup.players >= 2 && setup.players <= max_players;
}

} // namespace ce::core
