#include "core/input/input_frame.hpp"
#include "core/minigames/circus/game/game.hpp"
#include "core/minigames/circus/world/world.hpp"
#include "core/time/fixed_step.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <string_view>
#include <vector>

using ce::core::ButtonsDown;
using ce::core::FixedStep;
using ce::core::GameButton;
using ce::core::InputFrame;
using ce::core::InputTracker;
using ce::core::circus::Campaign;
using ce::core::circus::ElephantState;
using ce::core::circus::EventKind;
using ce::core::circus::Game;
using ce::core::circus::GamePhase;
using ce::core::circus::Hoop;
using ce::core::circus::LevelData;
using ce::core::circus::Tunables;
using ce::core::circus::World;

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

LevelData level_with_hoop(double hoop_x)
{
    LevelData data = level(30.0, 2.0);
    data.objects.push_back(Hoop{hoop_x});
    return data;
}

// Buttons down by their letters: down("ax") is A and X.
ButtonsDown down(std::string_view letters)
{
    ButtonsDown buttons{};
    for (const char letter : letters)
    {
        const auto button = ce::core::button_from_name(std::string(1, letter));
        buttons[static_cast<std::size_t>(button.value())] = true;
    }
    return buttons;
}

int steps_in(double seconds)
{
    return std::max(1, static_cast<int>(std::lround(seconds / dt)));
}

// Plays a world step by step and remembers what happened.
struct Player
{
    World& world;
    InputTracker tracker{};
    std::vector<EventKind> events{};
    double highest = 0.0;

    // Holds the given buttons for the given time (at least one step).
    void run(double seconds, std::string_view letters)
    {
        for (int i = 0; i < steps_in(seconds); ++i)
        {
            for (const auto& event : world.step(tracker.next(down(letters)), dt))
            {
                events.push_back(event.kind);
            }
            highest = std::max(highest, world.elephant().y);
        }
    }

    // X, then B one step later, while holding the other given buttons.
    void jump(std::string_view also_held)
    {
        const std::string held{also_held};
        run(dt, held + "x");
        run(dt, held);
        run(dt, held + "b");
    }

    bool saw(EventKind kind) const
    {
        return std::find(events.begin(), events.end(), kind) != events.end();
    }
};

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
    const InputFrame idle = tracker.next(down(""));
    EXPECT_FALSE(idle.pressed(GameButton::X));
    EXPECT_FALSE(idle.held(GameButton::X));

    const InputFrame press = tracker.next(down("x"));
    EXPECT_TRUE(press.pressed(GameButton::X));
    EXPECT_TRUE(press.held(GameButton::X));
    EXPECT_FALSE(press.held(GameButton::A));

    const InputFrame hold = tracker.next(down("ax"));
    EXPECT_FALSE(hold.pressed(GameButton::X));
    EXPECT_TRUE(hold.held(GameButton::X));
    EXPECT_TRUE(hold.pressed(GameButton::A));

    const InputFrame release = tracker.next(down("a"));
    EXPECT_TRUE(release.released(GameButton::X));
    EXPECT_FALSE(release.held(GameButton::X));
    EXPECT_FALSE(release.released(GameButton::A));

    EXPECT_FALSE(tracker.next(down("a")).released(GameButton::X));
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

TEST(Tunables, JumpReachesItsHeight)
{
    const Tunables tunables;
    const double v = tunables.jump_speed();
    EXPECT_NEAR(v * v / (2.0 * tunables.gravity), tunables.jump_height, 1e-9);
    EXPECT_NEAR(tunables.air_time(), 2.0 * v / tunables.gravity, 1e-9);
}

TEST(TouchesHoop, OnlyTheOpeningIsFree)
{
    const Hoop hoop{10.0};
    const double r = 0.45;
    EXPECT_FALSE(touches_hoop(hoop, 8.0, r, r));
    EXPECT_TRUE(touches_hoop(hoop, 10.0, r, r));
    EXPECT_TRUE(touches_hoop(hoop, 9.7, r, r));
    EXPECT_FALSE(touches_hoop(hoop, 10.0, hoop.height, r));
    EXPECT_TRUE(touches_hoop(hoop, 10.0, hoop.top() - 0.2, r));
    EXPECT_FALSE(touches_hoop(hoop, 10.5, r, r));
}

TEST(World, ElephantStartsAtTheLevelStart)
{
    const World world{level(30.0, 2.0)};
    EXPECT_DOUBLE_EQ(world.elephant().x, 2.0);
    EXPECT_DOUBLE_EQ(world.elephant().y, 0.0);
    EXPECT_EQ(world.elephant().state, ElephantState::Standing);
    EXPECT_DOUBLE_EQ(world.time(), 0.0);
}

TEST(World, StandsStillUntilAIsHeld)
{
    World world{level(30.0, 2.0)};
    Player player{world};
    player.run(0.5, "");
    player.jump("");
    player.run(0.5, "y");
    player.run(0.5, "bxy");
    EXPECT_TRUE(player.events.empty());
    EXPECT_EQ(world.elephant().state, ElephantState::Standing);
    EXPECT_DOUBLE_EQ(world.elephant().x, 2.0);
    EXPECT_FALSE(world.failed());
}

TEST(World, HoldingAWalksOnTwoLegs)
{
    World world{level(30.0, 2.0)};
    Player player{world};
    player.run(dt, "a");
    EXPECT_EQ(world.elephant().state, ElephantState::Walking);
    player.run(1.0, "a");
    EXPECT_NEAR(world.elephant().x, 2.0 + world.tunables().walk_speed, 1e-9);
    EXPECT_TRUE(player.events.empty());
}

TEST(World, LettingGoOfAFallsAfterAShortWhile)
{
    World world{level(30.0, 2.0)};
    Player player{world};
    player.run(1.0, "a");
    player.run(0.29, "");
    EXPECT_EQ(world.elephant().state, ElephantState::Walking);
    player.run(0.05, "");
    EXPECT_EQ(world.elephant().state, ElephantState::Fallen);
    EXPECT_TRUE(world.failed());
    EXPECT_TRUE(player.saw(EventKind::Fell));

    const double x = world.elephant().x;
    EXPECT_TRUE(world.step(InputFrame{}, dt).empty());
    EXPECT_DOUBLE_EQ(world.elephant().x, x);
}

TEST(World, XThenBJumpsWithASalto)
{
    World world{level(30.0, 2.0)};
    Player player{world};
    player.run(0.5, "a");
    player.jump("a");
    EXPECT_TRUE(player.saw(EventKind::Jumped));
    EXPECT_EQ(world.elephant().state, ElephantState::Jumping);

    const double half = world.tunables().air_time() / 2.0;
    player.run(half, "a");
    EXPECT_NEAR(world.elephant().salto, 0.5, 0.02);
    EXPECT_FALSE(player.saw(EventKind::Landed));

    player.run(half, "a");
    EXPECT_TRUE(player.saw(EventKind::Landed));
    EXPECT_EQ(world.elephant().state, ElephantState::Walking);
    EXPECT_DOUBLE_EQ(world.elephant().y, 0.0);
    EXPECT_NEAR(player.highest, world.tunables().jump_height, 0.05);
}

TEST(World, NoNeedToHoldAInTheAir)
{
    World world{level(30.0, 2.0)};
    Player player{world};
    player.run(0.5, "a");
    player.jump("");
    player.run(world.tunables().air_time() - 0.05, "");
    EXPECT_EQ(world.elephant().state, ElephantState::Jumping);
    player.run(0.1, "");
    EXPECT_TRUE(player.saw(EventKind::Landed));
    EXPECT_FALSE(world.failed());
    player.run(world.tunables().fall_grace + 0.05, "");
    EXPECT_TRUE(player.saw(EventKind::Fell));
}

TEST(World, WaitingForBAfterXKeepsTheElephantUp)
{
    World world{level(30.0, 2.0)};
    Player player{world};
    player.run(0.5, "a");
    player.run(dt, "x");
    player.run(0.35, "");
    EXPECT_EQ(world.elephant().state, ElephantState::Walking);
    player.run(dt, "b");
    EXPECT_TRUE(player.saw(EventKind::Jumped));
}

TEST(World, LateBDoesNotJump)
{
    World world{level(30.0, 2.0)};
    Player player{world};
    player.run(0.5, "a");
    player.run(dt, "ax");
    player.run(world.tunables().salto_window + 0.05, "a");
    player.run(dt, "ab");
    EXPECT_FALSE(player.saw(EventKind::Jumped));
    EXPECT_EQ(world.elephant().state, ElephantState::Walking);
}

TEST(World, WalkingIntoAHoopTouchesIt)
{
    World world{level_with_hoop(14.0)};
    Player player{world};
    player.run(6.0, "a");
    EXPECT_TRUE(player.saw(EventKind::HoopTouched));
    EXPECT_FALSE(player.saw(EventKind::HoopPassed));
    EXPECT_TRUE(world.failed());
    EXPECT_LT(world.elephant().x, 14.0);
}

TEST(World, JumpingThroughAHoopPassesIt)
{
    World world{level_with_hoop(14.0)};
    Player player{world};
    // Take off so that the top of the jump is right at the hoop.
    const Tunables& t = world.tunables();
    const double take_off = 14.0 - t.walk_speed * t.air_time() / 2.0;
    player.run((take_off - 2.0) / t.walk_speed, "a");
    player.jump("a");
    player.run(2.0, "a");
    EXPECT_TRUE(player.saw(EventKind::Jumped));
    EXPECT_TRUE(player.saw(EventKind::HoopPassed));
    EXPECT_TRUE(player.saw(EventKind::Landed));
    EXPECT_FALSE(player.saw(EventKind::HoopTouched));
    EXPECT_FALSE(world.failed());
    EXPECT_GT(world.elephant().x, 15.0);
}

TEST(World, JumpingTooEarlyTouchesTheHoop)
{
    World world{level_with_hoop(14.0)};
    Player player{world};
    player.run(2.5, "a");
    player.jump("a");
    player.run(3.0, "a");
    EXPECT_TRUE(player.saw(EventKind::HoopTouched));
    EXPECT_TRUE(world.failed());
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

TEST(Game, AAfterTheLevelMovesOn)
{
    Game game{Campaign{{"a", "b"}}, loader({{"a", level(10.0, 10.0)}, {"b", level(20.0, 5.0)}})};
    InputTracker tracker;
    game.step(tracker.next(down("")), dt);
    ASSERT_EQ(game.phase(), GamePhase::LevelComplete);
    game.step(tracker.next(down("bxy")), dt);
    EXPECT_EQ(game.phase(), GamePhase::LevelComplete);
    game.step(tracker.next(down("abxy")), dt);
    EXPECT_EQ(game.phase(), GamePhase::Playing);
    EXPECT_EQ(game.level_index(), 1u);
    game.step(tracker.next(down("a")), dt);
    EXPECT_EQ(game.level_index(), 1u);
}

TEST(Game, AFallStartsTheLevelOver)
{
    Game game{Campaign{{"a"}}, loader({{"a", level(30.0, 2.0)}})};
    InputTracker tracker;
    auto run = [&](double seconds, std::string_view letters)
    {
        for (int i = 0; i < steps_in(seconds); ++i)
        {
            game.step(tracker.next(down(letters)), dt);
        }
    };
    run(0.5, "a");
    run(0.4, "");
    ASSERT_EQ(game.phase(), GamePhase::LevelFailed);
    EXPECT_TRUE(game.world().failed());

    run(0.9, "a");
    EXPECT_EQ(game.phase(), GamePhase::LevelFailed);
    run(0.2, "");
    EXPECT_EQ(game.phase(), GamePhase::Playing);
    EXPECT_EQ(game.world().elephant().state, ElephantState::Standing);
    EXPECT_DOUBLE_EQ(game.world().elephant().x, 2.0);
    EXPECT_LT(game.world().time(), 0.2);
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
