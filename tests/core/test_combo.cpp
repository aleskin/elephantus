#include "core/input/combo.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

using ce::core::Combo;
using ce::core::GameButton;
using ce::core::InputFrame;

namespace
{

constexpr double dt = 0.01;

InputFrame press(GameButton button)
{
    InputFrame frame;
    frame.set(button, false, true);
    return frame;
}

Combo x_then_b()
{
    return Combo{{GameButton::X, GameButton::B}, 0.1};
}

} // namespace

TEST(Combo, FiresWhenTheButtonsComeInOrder)
{
    Combo combo = x_then_b();
    EXPECT_FALSE(combo.pending());
    EXPECT_FALSE(combo.step(press(GameButton::X), dt));
    EXPECT_TRUE(combo.pending());
    EXPECT_FALSE(combo.step(InputFrame{}, dt));
    EXPECT_TRUE(combo.step(press(GameButton::B), dt));
    EXPECT_FALSE(combo.pending());
}

TEST(Combo, WrongOrderDoesNotCount)
{
    Combo combo = x_then_b();
    EXPECT_FALSE(combo.step(press(GameButton::B), dt));
    EXPECT_FALSE(combo.pending());
    EXPECT_FALSE(combo.step(press(GameButton::X), dt));
    EXPECT_FALSE(combo.step(press(GameButton::A), dt));
    EXPECT_FALSE(combo.pending());
    EXPECT_FALSE(combo.step(press(GameButton::B), dt));
}

TEST(Combo, PressingTheFirstButtonAgainStartsOver)
{
    Combo combo = x_then_b();
    combo.step(press(GameButton::X), dt);
    EXPECT_FALSE(combo.step(press(GameButton::X), dt));
    EXPECT_TRUE(combo.pending());
    EXPECT_TRUE(combo.step(press(GameButton::B), dt));
}

TEST(Combo, TooSlowDoesNotCount)
{
    Combo combo = x_then_b();
    combo.step(press(GameButton::X), dt);
    for (int i = 0; i < 11; ++i)
    {
        combo.step(InputFrame{}, dt);
    }
    EXPECT_FALSE(combo.pending());
    EXPECT_FALSE(combo.step(press(GameButton::B), dt));
}

TEST(Combo, HeldButtonsDoNotCountAgain)
{
    Combo combo = x_then_b();
    InputFrame hold;
    hold.set(GameButton::X, true, true);
    EXPECT_FALSE(combo.step(hold, dt));
    EXPECT_FALSE(combo.pending());
}

TEST(Combo, EmptyIsAnError)
{
    EXPECT_THROW((Combo{{}, 0.1}), std::invalid_argument);
}
