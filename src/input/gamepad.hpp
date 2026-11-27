#pragma once

#include "core/input/controls.hpp"
#include "core/input/input_frame.hpp"

#include <string>

namespace ce::input
{

// Adds the game buttons held on any connected gamepad to `down`, with the gamepad buttons
// from the controls. Works together with the keyboard: a button counts when it is held on
// either.
void read_gamepads(const core::Controls& controls, core::ButtonsDown& down);

// Names of the connected gamepads, "" when there are none; for the log when it changes.
std::string gamepad_names();

} // namespace ce::input
