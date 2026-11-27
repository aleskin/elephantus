#pragma once

#include "core/input/controls.hpp"
#include "core/input/input_frame.hpp"

namespace ce::input
{

// Which game buttons are held on the keyboard right now, with the keys from the controls.
core::ButtonsDown read_keyboard(const core::Controls& controls);

} // namespace ce::input
