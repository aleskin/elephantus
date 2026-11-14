#include "input/keyboard.hpp"

#include <raylib.h>

#include <array>
#include <cstddef>

namespace ce::input
{

namespace
{

struct Binding
{
    core::GameButton button;
    std::array<int, 2> keys;
};

// TODO(CEL-00008): temporary keys until Polina picks hers; later from the settings file.
constexpr std::array<Binding, core::game_button_count> bindings = {{
    {core::GameButton::TwoLegs, {KEY_LEFT_SHIFT, KEY_UP}},
    {core::GameButton::Jump, {KEY_SPACE, KEY_RIGHT_CONTROL}},
}};

} // namespace

core::ButtonsDown read_keyboard()
{
    core::ButtonsDown down{};
    for (const Binding& binding : bindings)
    {
        for (const int key : binding.keys)
        {
            if (IsKeyDown(key))
            {
                down[static_cast<std::size_t>(binding.button)] = true;
            }
        }
    }
    return down;
}

} // namespace ce::input
