#include "core/input/pad_layout.hpp"

namespace ce::core
{

GameButton button_at(PadLayout layout, PadFace face)
{
    switch (face)
    {
    case PadFace::Right:
        return layout == PadLayout::Nintendo ? GameButton::A : GameButton::B;
    case PadFace::Down:
        return layout == PadLayout::Nintendo ? GameButton::B : GameButton::A;
    case PadFace::Up:
        return layout == PadLayout::Nintendo ? GameButton::X : GameButton::Y;
    case PadFace::Left:
        return layout == PadLayout::Nintendo ? GameButton::Y : GameButton::X;
    }
    return GameButton::A;
}

std::optional<PadLayout> pad_layout_from_name(std::string_view name)
{
    if (name == "nintendo")
    {
        return PadLayout::Nintendo;
    }
    if (name == "xbox")
    {
        return PadLayout::Xbox;
    }
    return std::nullopt;
}

} // namespace ce::core
