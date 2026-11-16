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

struct FaceButton
{
    core::PadFace face;
    int button;
};

constexpr std::array<FaceButton, 4> faces = {{
    {core::PadFace::Right, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT},
    {core::PadFace::Down, GAMEPAD_BUTTON_RIGHT_FACE_DOWN},
    {core::PadFace::Up, GAMEPAD_BUTTON_RIGHT_FACE_UP},
    {core::PadFace::Left, GAMEPAD_BUTTON_RIGHT_FACE_LEFT},
}};

} // namespace

void read_gamepads(core::PadLayout layout, core::ButtonsDown& down)
{
    for (int pad = 0; pad < max_gamepads; ++pad)
    {
        if (!IsGamepadAvailable(pad))
        {
            continue;
        }
        for (const FaceButton& face : faces)
        {
            if (IsGamepadButtonDown(pad, face.button))
            {
                down[static_cast<std::size_t>(core::button_at(layout, face.face))] = true;
            }
        }
    }
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
