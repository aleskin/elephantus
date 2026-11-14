#include "core/game_info.hpp"

#include <gtest/gtest.h>

TEST(GameInfo, TitleIsCircusElephant)
{
    EXPECT_EQ(ce::core::game_title(), "Circus Elephant");
}
