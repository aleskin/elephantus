#include "core/input/combo.hpp"

#include <stdexcept>
#include <utility>

namespace ce::core
{

namespace
{

constexpr GameButton all_buttons[] = {GameButton::A, GameButton::B, GameButton::X, GameButton::Y};

} // namespace

Combo::Combo(std::vector<GameButton> sequence, double window)
    : sequence_(std::move(sequence))
    , window_(window)
{
    if (sequence_.empty())
    {
        throw std::invalid_argument("a combo needs at least one button");
    }
}

bool Combo::step(const InputFrame& input, double dt)
{
    if (next_ > 0)
    {
        since_last_ += dt;
        if (since_last_ > window_)
        {
            reset();
        }
    }
    for (const GameButton button : all_buttons)
    {
        if (!input.pressed(button))
        {
            continue;
        }
        if (button != sequence_[next_])
        {
            reset();
            if (button != sequence_[0])
            {
                continue;
            }
        }
        ++next_;
        since_last_ = 0.0;
        if (next_ == sequence_.size())
        {
            reset();
            return true;
        }
    }
    return false;
}

bool Combo::pending() const
{
    return next_ > 0;
}

void Combo::set_window(double window)
{
    window_ = window;
}

void Combo::reset()
{
    next_ = 0;
    since_last_ = 0.0;
}

} // namespace ce::core
