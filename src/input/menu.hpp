#pragma once

#include "core/input/pad_layout.hpp"
#include "core/party/flow.hpp"

namespace ce::input
{

// Menu buttons held on the keyboard and on any gamepad. Keyboard: arrows; A, Enter or Space
// choose; B, Esc or Backspace go back. Gamepad: d-pad or left stick; game buttons A and B
// at their places for the layout.
core::MenuButtonsDown read_menu_buttons(core::PadLayout layout);

// Esc on the keyboard or Start on a gamepad, pressed in this frame: leave the mini-game.
bool leave_pressed();

} // namespace ce::input
