#pragma once

#include "core/input/input_frame.hpp"
#include "core/input/pad_layout.hpp"

#include <string>

namespace ce::input
{

// Adds the game buttons held on any connected gamepad to `down`. Works together with the
// keyboard: a button counts when it is held on either.
void read_gamepads(core::PadLayout layout, core::ButtonsDown& down);

// Names of the connected gamepads, "" when there are none; for the log when it changes.
std::string gamepad_names();

} // namespace ce::input
