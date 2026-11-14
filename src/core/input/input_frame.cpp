#include "core/input/input_frame.hpp"

namespace ce::core
{

namespace
{

std::size_t index(GameButton button)
{
    return static_cast<std::size_t>(button);
}

} // namespace

bool InputFrame::pressed(GameButton button) const
{
    return pressed_[index(button)];
}

bool InputFrame::held(GameButton button) const
{
    return held_[index(button)];
}

bool InputFrame::released(GameButton button) const
{
    return released_[index(button)];
}

void InputFrame::set(GameButton button, bool was_down, bool is_down)
{
    pressed_[index(button)] = is_down && !was_down;
    held_[index(button)] = is_down;
    released_[index(button)] = was_down && !is_down;
}

InputFrame InputTracker::next(const ButtonsDown& down)
{
    InputFrame frame;
    for (std::size_t i = 0; i < game_button_count; ++i)
    {
        frame.set(static_cast<GameButton>(i), previous_[i], down[i]);
    }
    previous_ = down;
    return frame;
}

} // namespace ce::core
