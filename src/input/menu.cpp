#include "input/menu.hpp"

#include <raylib.h>

#include <array>
#include <cstddef>

namespace ce::input
{

namespace
{

constexpr int max_gamepads = 4;
// How far the left stick has to be pushed to count as a d-pad press.
constexpr float stick_threshold = 0.5F;

struct KeyBinding
{
    core::MenuButton button;
    int key;
};

constexpr std::array<KeyBinding, 11> keys = {{
    {core::MenuButton::Up, KEY_UP},
    {core::MenuButton::Down, KEY_DOWN},
    {core::MenuButton::Left, KEY_LEFT},
    {core::MenuButton::Right, KEY_RIGHT},
    {core::MenuButton::Confirm, KEY_A},
    {core::MenuButton::Confirm, KEY_ENTER},
    {core::MenuButton::Confirm, KEY_SPACE},
    {core::MenuButton::Confirm, KEY_KP_ENTER},
    {core::MenuButton::Back, KEY_B},
    {core::MenuButton::Back, KEY_ESCAPE},
    {core::MenuButton::Back, KEY_BACKSPACE},
}};

struct PadBinding
{
    core::MenuButton button;
    int pad_button;
};

constexpr std::array<PadBinding, 4> dpad = {{
    {core::MenuButton::Up, GAMEPAD_BUTTON_LEFT_FACE_UP},
    {core::MenuButton::Down, GAMEPAD_BUTTON_LEFT_FACE_DOWN},
    {core::MenuButton::Left, GAMEPAD_BUTTON_LEFT_FACE_LEFT},
    {core::MenuButton::Right, GAMEPAD_BUTTON_LEFT_FACE_RIGHT},
}};

struct FaceButton
{
    core::PadFace face;
    int pad_button;
};

constexpr std::array<FaceButton, 4> faces = {{
    {core::PadFace::Right, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT},
    {core::PadFace::Down, GAMEPAD_BUTTON_RIGHT_FACE_DOWN},
    {core::PadFace::Up, GAMEPAD_BUTTON_RIGHT_FACE_UP},
    {core::PadFace::Left, GAMEPAD_BUTTON_RIGHT_FACE_LEFT},
}};

void set(core::MenuButtonsDown& down, core::MenuButton button)
{
    down[static_cast<std::size_t>(button)] = true;
}

} // namespace

core::MenuButtonsDown read_menu_buttons(core::PadLayout layout)
{
    core::MenuButtonsDown down{};
    for (const KeyBinding& binding : keys)
    {
        if (IsKeyDown(binding.key))
        {
            set(down, binding.button);
        }
    }
    for (int pad = 0; pad < max_gamepads; ++pad)
    {
        if (!IsGamepadAvailable(pad))
        {
            continue;
        }
        for (const PadBinding& binding : dpad)
        {
            if (IsGamepadButtonDown(pad, binding.pad_button))
            {
                set(down, binding.button);
            }
        }
        const float x = GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_X);
        const float y = GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y);
        if (y < -stick_threshold)
        {
            set(down, core::MenuButton::Up);
        }
        if (y > stick_threshold)
        {
            set(down, core::MenuButton::Down);
        }
        if (x < -stick_threshold)
        {
            set(down, core::MenuButton::Left);
        }
        if (x > stick_threshold)
        {
            set(down, core::MenuButton::Right);
        }
        for (const FaceButton& face : faces)
        {
            if (!IsGamepadButtonDown(pad, face.pad_button))
            {
                continue;
            }
            const core::GameButton button = core::button_at(layout, face.face);
            if (button == core::GameButton::A)
            {
                set(down, core::MenuButton::Confirm);
            }
            else if (button == core::GameButton::B)
            {
                set(down, core::MenuButton::Back);
            }
        }
    }
    return down;
}

bool leave_pressed()
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        return true;
    }
    for (int pad = 0; pad < max_gamepads; ++pad)
    {
        if (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_MIDDLE_RIGHT))
        {
            return true;
        }
    }
    return false;
}

} // namespace ce::input
