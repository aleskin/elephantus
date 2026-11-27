#pragma once

#include "core/input/controls.hpp"
#include "core/minigames/circus/game/game.hpp"

namespace ce::render::circus
{

// Text over the scene: which keys to press (`labels`, as the player set them), the level
// number, and what just happened (fell — the level starts over, the level is done — press A).
void draw_hud(const core::circus::Game& game, const core::ButtonLabels& labels);

} // namespace ce::render::circus
