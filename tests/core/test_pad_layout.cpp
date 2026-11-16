#include "core/input/pad_layout.hpp"

#include <gtest/gtest.h>

using ce::core::button_at;
using ce::core::GameButton;
using ce::core::pad_layout_from_name;
using ce::core::PadFace;
using ce::core::PadLayout;

TEST(PadLayout, NintendoLikePolinaSays)
{
    EXPECT_EQ(button_at(PadLayout::Nintendo, PadFace::Right), GameButton::A);
    EXPECT_EQ(button_at(PadLayout::Nintendo, PadFace::Down), GameButton::B);
    EXPECT_EQ(button_at(PadLayout::Nintendo, PadFace::Up), GameButton::X);
    EXPECT_EQ(button_at(PadLayout::Nintendo, PadFace::Left), GameButton::Y);
}

TEST(PadLayout, Xbox)
{
    EXPECT_EQ(button_at(PadLayout::Xbox, PadFace::Right), GameButton::B);
    EXPECT_EQ(button_at(PadLayout::Xbox, PadFace::Down), GameButton::A);
    EXPECT_EQ(button_at(PadLayout::Xbox, PadFace::Up), GameButton::Y);
    EXPECT_EQ(button_at(PadLayout::Xbox, PadFace::Left), GameButton::X);
}

TEST(PadLayout, Names)
{
    EXPECT_EQ(pad_layout_from_name("nintendo"), PadLayout::Nintendo);
    EXPECT_EQ(pad_layout_from_name("xbox"), PadLayout::Xbox);
    EXPECT_FALSE(pad_layout_from_name("Xbox"));
    EXPECT_FALSE(pad_layout_from_name(""));
}
