#pragma once

#include "core/input/controls_menu.hpp"
#include "core/input/pad_layout.hpp"
#include "core/party/flow.hpp"

namespace ce::render
{

// The screens around the mini-games, one file each. They draw the whole frame.

// TODO(CEL-00035): what Polina wants to see on the title; for now the name and a grey
// elephant. `time` is seconds since the title appeared, for the blinking "press A".
void draw_title(double time);
void draw_main_menu(const core::PartyFlow& flow);
void draw_minigame_list(const core::PartyFlow& flow);
// "Settings" -> "Controls": actions with their keys and gamepad buttons.
void draw_controls(const core::ControlsMenu& menu, core::PadLayout layout);

} // namespace ce::render
