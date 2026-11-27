#include "core/input/controls_menu.hpp"

#include <utility>

namespace ce::core
{

ControlsMenu::ControlsMenu(Controls controls, PadLayout layout)
    : controls_(controls)
    , layout_(layout)
{
}

const Controls& ControlsMenu::controls() const
{
    return controls_;
}

std::size_t ControlsMenu::row() const
{
    return row_;
}

ControlsColumn ControlsMenu::column() const
{
    return column_;
}

std::optional<GameButton> ControlsMenu::button() const
{
    if (on_reset())
    {
        return std::nullopt;
    }
    return controls_menu_rows[row_];
}

bool ControlsMenu::on_reset() const
{
    return row_ == controls_menu_row_count - 1;
}

bool ControlsMenu::waiting() const
{
    return waiting_;
}

void ControlsMenu::input(const MenuInput& input)
{
    if (waiting_)
    {
        return;
    }
    if (input.up)
    {
        row_ = (row_ + controls_menu_row_count - 1) % controls_menu_row_count;
    }
    if (input.down)
    {
        row_ = (row_ + 1) % controls_menu_row_count;
    }
    if (input.left)
    {
        column_ = ControlsColumn::Keyboard;
    }
    if (input.right)
    {
        column_ = ControlsColumn::Gamepad;
    }
    if (input.confirm)
    {
        if (on_reset())
        {
            const Controls defaults = default_controls(layout_);
            if (defaults != controls_)
            {
                changed_ = true;
            }
            controls_ = defaults;
        }
        else
        {
            waiting_ = true;
        }
    }
}

void ControlsMenu::assign_key(int key)
{
    if (!waiting_)
    {
        return;
    }
    if (key == keys::escape)
    {
        cancel();
        return;
    }
    if (column_ != ControlsColumn::Keyboard)
    {
        return;
    }
    waiting_ = false;
    Binding& mine = controls_[*button()];
    if (mine.key == key)
    {
        return;
    }
    for (Binding& other : controls_.buttons)
    {
        if (&other != &mine && other.key == key)
        {
            other.key = mine.key;
        }
    }
    mine.key = key;
    changed_ = true;
}

void ControlsMenu::assign_pad(PadButton pad)
{
    if (!waiting_)
    {
        return;
    }
    if (column_ != ControlsColumn::Gamepad)
    {
        return;
    }
    waiting_ = false;
    Binding& mine = controls_[*button()];
    if (mine.pad == pad)
    {
        return;
    }
    for (Binding& other : controls_.buttons)
    {
        if (&other != &mine && other.pad == pad)
        {
            other.pad = mine.pad;
        }
    }
    mine.pad = pad;
    changed_ = true;
}

void ControlsMenu::cancel()
{
    waiting_ = false;
}

bool ControlsMenu::take_changed()
{
    return std::exchange(changed_, false);
}

} // namespace ce::core
