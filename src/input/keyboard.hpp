#pragma once

#include "core/input/controls.hpp"
#include "core/input/input_frame.hpp"

#include <optional>

namespace ce::input
{

// Which game buttons are held on the keyboard right now, with the keys from the controls.
core::ButtonsDown read_keyboard(const core::Controls& controls);

// The first key pressed since the last call, for "Settings" -> "Controls". Call it every
// frame on that screen so that old presses do not wait in the queue.
std::optional<int> pressed_key();

} // namespace ce::input
