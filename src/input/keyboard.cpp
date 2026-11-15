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
    std::array<int, 1> keys;
};

// Keys with the same letters as the buttons Polina names (SPEC.md, "Buttons").
constexpr std::array<Binding, core::game_button_count> bindings = {{
    {core::GameButton::A, {KEY_A}},
    {core::GameButton::B, {KEY_B}},
    {core::GameButton::X, {KEY_X}},
    {core::GameButton::Y, {KEY_Y}},
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
