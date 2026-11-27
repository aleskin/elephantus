#include "core/input/controls_menu.hpp"

#include <gtest/gtest.h>

using ce::core::ControlsColumn;
using ce::core::ControlsMenu;
using ce::core::default_controls;
using ce::core::GameButton;
using ce::core::MenuInput;
using ce::core::PadButton;
using ce::core::PadLayout;
namespace keys = ce::core::keys;

namespace
{

MenuInput up()
{
    MenuInput input;
    input.up = true;
    return input;
}

MenuInput down()
{
    MenuInput input;
    input.down = true;
    return input;
}

MenuInput right()
{
    MenuInput input;
    input.right = true;
    return input;
}

MenuInput left()
{
    MenuInput input;
    input.left = true;
    return input;
}

MenuInput confirm()
{
    MenuInput input;
    input.confirm = true;
    return input;
}

ControlsMenu fresh()
{
    return ControlsMenu{default_controls(PadLayout::Nintendo), PadLayout::Nintendo};
}

} // namespace

TEST(ControlsMenu, RowsGoWalkJump1Jump2YThenReset)
{
    ControlsMenu menu = fresh();
    EXPECT_EQ(menu.button(), GameButton::A);
    menu.input(down());
    EXPECT_EQ(menu.button(), GameButton::X);
    menu.input(down());
    EXPECT_EQ(menu.button(), GameButton::B);
    menu.input(down());
    EXPECT_EQ(menu.button(), GameButton::Y);
    menu.input(down());
    EXPECT_TRUE(menu.on_reset());
    EXPECT_FALSE(menu.button().has_value());
    menu.input(down());
    EXPECT_EQ(menu.button(), GameButton::A);
    menu.input(up());
    EXPECT_TRUE(menu.on_reset());
}

TEST(ControlsMenu, LeftAndRightChooseTheColumn)
{
    ControlsMenu menu = fresh();
    EXPECT_EQ(menu.column(), ControlsColumn::Keyboard);
    menu.input(right());
    EXPECT_EQ(menu.column(), ControlsColumn::Gamepad);
    menu.input(right());
    EXPECT_EQ(menu.column(), ControlsColumn::Gamepad);
    menu.input(left());
    EXPECT_EQ(menu.column(), ControlsColumn::Keyboard);
}

TEST(ControlsMenu, ConfirmWaitsAndTheNextKeyIsTaken)
{
    ControlsMenu menu = fresh();
    menu.input(confirm());
    EXPECT_TRUE(menu.waiting());
    menu.input(down());
    EXPECT_EQ(menu.button(), GameButton::A);
    menu.assign_key(keys::up);
    EXPECT_FALSE(menu.waiting());
    EXPECT_EQ(menu.controls()[GameButton::A].key, keys::up);
    EXPECT_TRUE(menu.take_changed());
    EXPECT_FALSE(menu.take_changed());
}

TEST(ControlsMenu, TakenKeyIsSwapped)
{
    ControlsMenu menu = fresh();
    menu.input(confirm());
    menu.assign_key(keys::left);
    EXPECT_EQ(menu.controls()[GameButton::A].key, keys::left);
    EXPECT_EQ(menu.controls()[GameButton::X].key, keys::a);
}

TEST(ControlsMenu, EscapeCancelsAndIsNeverBound)
{
    ControlsMenu menu = fresh();
    menu.input(confirm());
    menu.assign_key(keys::escape);
    EXPECT_FALSE(menu.waiting());
    EXPECT_EQ(menu.controls()[GameButton::A].key, keys::a);
    EXPECT_FALSE(menu.take_changed());
}

TEST(ControlsMenu, SameKeyIsNoChange)
{
    ControlsMenu menu = fresh();
    menu.input(confirm());
    menu.assign_key(keys::a);
    EXPECT_FALSE(menu.waiting());
    EXPECT_FALSE(menu.take_changed());
}

TEST(ControlsMenu, GamepadColumnTakesPadButtonsAndSwaps)
{
    ControlsMenu menu = fresh();
    menu.input(right());
    menu.input(confirm());
    menu.assign_key(keys::up);
    EXPECT_TRUE(menu.waiting());
    menu.assign_pad(PadButton::R1);
    EXPECT_FALSE(menu.waiting());
    EXPECT_EQ(menu.controls()[GameButton::A].pad, PadButton::R1);
    EXPECT_TRUE(menu.take_changed());

    const PadButton x_pad = menu.controls()[GameButton::X].pad;
    menu.input(confirm());
    menu.assign_pad(x_pad);
    EXPECT_EQ(menu.controls()[GameButton::A].pad, x_pad);
    EXPECT_EQ(menu.controls()[GameButton::X].pad, PadButton::R1);
}

TEST(ControlsMenu, KeyboardColumnIgnoresPadButtons)
{
    ControlsMenu menu = fresh();
    menu.input(confirm());
    menu.assign_pad(PadButton::R1);
    EXPECT_TRUE(menu.waiting());
    menu.cancel();
    EXPECT_FALSE(menu.waiting());
    EXPECT_EQ(menu.controls(), default_controls(PadLayout::Nintendo));
}

TEST(ControlsMenu, ResetBringsBackTheDefaults)
{
    ControlsMenu menu = fresh();
    menu.input(confirm());
    menu.assign_key(keys::up);
    EXPECT_TRUE(menu.take_changed());
    menu.input(up());
    EXPECT_TRUE(menu.on_reset());
    menu.input(confirm());
    EXPECT_FALSE(menu.waiting());
    EXPECT_EQ(menu.controls(), default_controls(PadLayout::Nintendo));
    EXPECT_TRUE(menu.take_changed());
    menu.input(confirm());
    EXPECT_FALSE(menu.take_changed());
}
