#include "input/gamepad.hpp"

#include <raylib.h>

#include <array>
#include <cstddef>

namespace ce::input
{

namespace
{

// raylib numbers gamepads from 0; four is plenty for one family.
constexpr int max_gamepads = 4;

// raylib numbers of the gamepad buttons, in the order of core::PadButton.
constexpr std::array<int, core::pad_button_count> raylib_buttons = {
    GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, GAMEPAD_BUTTON_RIGHT_FACE_UP,
    GAMEPAD_BUTTON_RIGHT_FACE_LEFT,  GAMEPAD_BUTTON_LEFT_FACE_UP,    GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    GAMEPAD_BUTTON_LEFT_FACE_LEFT,   GAMEPAD_BUTTON_LEFT_FACE_RIGHT, GAMEPAD_BUTTON_LEFT_TRIGGER_1,
    GAMEPAD_BUTTON_LEFT_TRIGGER_2,   GAMEPAD_BUTTON_RIGHT_TRIGGER_1, GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
    GAMEPAD_BUTTON_MIDDLE_LEFT,      GAMEPAD_BUTTON_LEFT_THUMB,      GAMEPAD_BUTTON_RIGHT_THUMB,
};

} // namespace

void read_gamepads(const core::Controls& controls, core::ButtonsDown& down)
{
    for (int pad = 0; pad < max_gamepads; ++pad)
    {
        if (!IsGamepadAvailable(pad))
        {
            continue;
        }
        for (std::size_t i = 0; i < core::game_button_count; ++i)
        {
            const auto button = static_cast<std::size_t>(controls.buttons[i].pad);
            if (IsGamepadButtonDown(pad, raylib_buttons[button]))
            {
                down[i] = true;
            }
        }
    }
}

std::optional<core::PadButton> pressed_pad_button()
{
    for (int pad = 0; pad < max_gamepads; ++pad)
    {
        if (!IsGamepadAvailable(pad))
        {
            continue;
        }
        for (std::size_t i = 0; i < core::pad_button_count; ++i)
        {
            if (IsGamepadButtonPressed(pad, raylib_buttons[i]))
            {
                return static_cast<core::PadButton>(i);
            }
        }
    }
    return std::nullopt;
}

std::string gamepad_names()
{
    std::string names;
    for (int pad = 0; pad < max_gamepads; ++pad)
    {
        if (!IsGamepadAvailable(pad))
        {
            continue;
        }
        if (!names.empty())
        {
            names += ", ";
        }
        names += std::to_string(pad) + ": " + GetGamepadName(pad);
    }
    return names;
}

} // namespace ce::input
