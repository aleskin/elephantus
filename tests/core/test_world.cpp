#include "core/game/game.hpp"
#include "core/input/input_frame.hpp"
#include "core/world/fixed_step.hpp"
#include "core/world/world.hpp"

#include <gtest/gtest.h>

#include <map>
#include <string>

using ce::core::ButtonsDown;
using ce::core::Campaign;
using ce::core::EventKind;
using ce::core::FixedStep;
using ce::core::Game;
using ce::core::GameButton;
using ce::core::GamePhase;
using ce::core::InputFrame;
using ce::core::InputTracker;
using ce::core::LevelData;
using ce::core::World;

namespace
{

constexpr double dt = 1.0 / 120.0;

LevelData level(double length, double start_x)
{
    LevelData data;
    data.title = "test";
    data.location = "arena";
    data.length = length;
    data.start_x = start_x;
    return data;
}

ButtonsDown down(bool two_legs, bool jump)
{
    return ButtonsDown{two_legs, jump};
}

// Levels by file name; a level whose start is at its end finishes on the first step.
Game::LevelLoader loader(const std::map<std::string, LevelData>& levels)
{
    return [levels](const std::filesystem::path& path)
    {
        return levels.at(path.string());
    };
}

} // namespace

TEST(InputTracker, PressHoldRelease)
{
    InputTracker tracker;
    const InputFrame idle = tracker.next(down(false, false));
    EXPECT_FALSE(idle.pressed(GameButton::Jump));
    EXPECT_FALSE(idle.held(GameButton::Jump));

    const InputFrame press = tracker.next(down(false, true));
    EXPECT_TRUE(press.pressed(GameButton::Jump));
    EXPECT_TRUE(press.held(GameButton::Jump));
    EXPECT_FALSE(press.held(GameButton::TwoLegs));

    const InputFrame hold = tracker.next(down(true, true));
    EXPECT_FALSE(hold.pressed(GameButton::Jump));
    EXPECT_TRUE(hold.held(GameButton::Jump));
    EXPECT_TRUE(hold.pressed(GameButton::TwoLegs));

    const InputFrame release = tracker.next(down(true, false));
    EXPECT_TRUE(release.released(GameButton::Jump));
    EXPECT_FALSE(release.held(GameButton::Jump));
    EXPECT_FALSE(release.released(GameButton::TwoLegs));

    EXPECT_FALSE(tracker.next(down(true, false)).released(GameButton::Jump));
}

TEST(FixedStep, CountsWholeStepsAndKeepsTheRest)
{
    FixedStep step{100.0};
    EXPECT_DOUBLE_EQ(step.dt(), 0.01);
    EXPECT_EQ(step.advance(0.025), 2);
    EXPECT_EQ(step.advance(0.004), 0);
    EXPECT_EQ(step.advance(0.001), 1);
    EXPECT_EQ(step.advance(-1.0), 0);
}

TEST(FixedStep, LongStallIsCapped)
{
    FixedStep step{8.0};
    EXPECT_EQ(step.advance(10.0), 2);
    EXPECT_EQ(step.advance(0.0), 0);
}

TEST(FixedStep, SixtyFramesPerSecondGiveTwoStepsEach)
{
    FixedStep step;
    int total = 0;
    for (int i = 0; i < 60; ++i)
    {
        total += step.advance(1.0 / 60.0);
    }
    EXPECT_NEAR(total, 120, 1);
}

TEST(World, ElephantStartsAtTheLevelStart)
{
    const World world{level(30.0, 2.0)};
    EXPECT_DOUBLE_EQ(world.elephant().x, 2.0);
    EXPECT_DOUBLE_EQ(world.elephant().y, 0.0);
    EXPECT_DOUBLE_EQ(world.time(), 0.0);
}

TEST(World, StandsStillAndCountsTime)
{
    World world{level(30.0, 2.0)};
    InputTracker tracker;
    for (int i = 0; i < 120; ++i)
    {
        EXPECT_TRUE(world.step(tracker.next(down(i % 2 == 0, i % 3 == 0)), dt).empty());
    }
    EXPECT_NEAR(world.time(), 1.0, 1e-9);
    EXPECT_DOUBLE_EQ(world.elephant().x, 2.0);
    EXPECT_FALSE(world.finished());
}

TEST(World, ReachingTheEndFinishesTheLevelOnce)
{
    World world{level(10.0, 10.0)};
    const auto first = world.step(InputFrame{}, dt);
    ASSERT_EQ(first.size(), 1u);
    EXPECT_EQ(first[0].kind, EventKind::LevelFinished);
    EXPECT_TRUE(world.finished());
    EXPECT_TRUE(world.step(InputFrame{}, dt).empty());
}

TEST(Game, GoesThroughLevelsToTheEnd)
{
    Game game{Campaign{{"a", "b"}}, loader({{"a", level(10.0, 10.0)}, {"b", level(20.0, 20.0)}})};
    EXPECT_EQ(game.level_count(), 2u);
    EXPECT_EQ(game.phase(), GamePhase::Playing);
    EXPECT_EQ(game.level_index(), 0u);

    game.next_level();
    EXPECT_EQ(game.level_index(), 0u);

    EXPECT_EQ(game.step(InputFrame{}, dt).size(), 1u);
    EXPECT_EQ(game.phase(), GamePhase::LevelComplete);
    EXPECT_TRUE(game.step(InputFrame{}, dt).empty());

    game.next_level();
    EXPECT_EQ(game.phase(), GamePhase::Playing);
    EXPECT_EQ(game.level_index(), 1u);
    EXPECT_DOUBLE_EQ(game.world().level().length, 20.0);

    game.step(InputFrame{}, dt);
    EXPECT_EQ(game.phase(), GamePhase::LevelComplete);
    game.next_level();
    EXPECT_EQ(game.phase(), GamePhase::CampaignComplete);
    EXPECT_EQ(game.level_index(), 1u);
}

TEST(Game, LevelInProgressStaysPlaying)
{
    Game game{Campaign{{"a"}}, loader({{"a", level(10.0, 1.0)}})};
    for (int i = 0; i < 240; ++i)
    {
        game.step(InputFrame{}, dt);
    }
    EXPECT_EQ(game.phase(), GamePhase::Playing);
    EXPECT_NEAR(game.world().time(), 2.0, 1e-9);
}

TEST(Game, EmptyCampaignIsAnError)
{
    EXPECT_THROW((Game{Campaign{}, loader({})}), std::invalid_argument);
}
