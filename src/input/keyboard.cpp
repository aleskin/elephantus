#include "input/keyboard.hpp"

#include <raylib.h>

#include <cstddef>

namespace ce::input
{

core::ButtonsDown read_keyboard(const core::Controls& controls)
{
    core::ButtonsDown down{};
    for (std::size_t i = 0; i < core::game_button_count; ++i)
    {
        down[i] = IsKeyDown(controls.buttons[i].key);
    }
    return down;
}

} // namespace ce::input
