#pragma once

#include "core/input/input_frame.hpp"

namespace ce::input
{

// Which game buttons are held on the keyboard right now.
core::ButtonsDown read_keyboard();

} // namespace ce::input
