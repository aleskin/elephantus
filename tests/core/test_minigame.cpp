#include "core/party/minigame.hpp"

#include <gtest/gtest.h>

using ce::core::fits;
using ce::core::max_players;
using ce::core::MinigameSetup;
using ce::core::PlayStyle;

TEST(Minigame, UpToFourPlayers)
{
    EXPECT_EQ(max_players, 4);
}

TEST(Minigame, TurnsMeansOnePlayerPerRun)
{
    EXPECT_TRUE(fits(PlayStyle::Turns, MinigameSetup{1}));
    EXPECT_FALSE(fits(PlayStyle::Turns, MinigameSetup{0}));
    EXPECT_FALSE(fits(PlayStyle::Turns, MinigameSetup{2}));
}

TEST(Minigame, TogetherMeansTwoToFour)
{
    EXPECT_FALSE(fits(PlayStyle::Together, MinigameSetup{1}));
    EXPECT_TRUE(fits(PlayStyle::Together, MinigameSetup{2}));
    EXPECT_TRUE(fits(PlayStyle::Together, MinigameSetup{4}));
    EXPECT_FALSE(fits(PlayStyle::Together, MinigameSetup{5}));
}
