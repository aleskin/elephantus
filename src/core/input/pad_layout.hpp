#pragma once

#include "core/input/input_frame.hpp"

#include <optional>
#include <string_view>

namespace ce::core
{

// The four buttons on the right of a gamepad, by where they are. Gamepads mark them
// differently (letters, PlayStation shapes), but the place is always the same.
enum class PadFace
{
    Right,
    Down,
    Up,
    Left,
};

// Which letter sits where. Nintendo, like Polina's buttons: A right, B down, X up,
// Y left. Xbox: A down, B right, X left, Y up. A PlayStation-shaped pad (Xtrike Me GP-43:
// circle, cross, triangle, square) has no letters and works with either.
enum class PadLayout
{
    Nintendo,
    Xbox,
};

GameButton button_at(PadLayout layout, PadFace face);

// Names used in the game config: "nintendo", "xbox".
std::optional<PadLayout> pad_layout_from_name(std::string_view name);

} // namespace ce::core
