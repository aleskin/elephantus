#pragma once

#include "core/minigames/circus/game/game.hpp"

namespace ce::render::circus
{

// Text over the scene: which buttons to press, the level number, and what just happened
// (fell — the level starts over, the level is done — press A). English until CEL-00034.
void draw_hud(const core::circus::Game& game);

} // namespace ce::render::circus
