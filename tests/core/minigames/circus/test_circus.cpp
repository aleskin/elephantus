#include "core/minigames/circus/circus.hpp"
#include "core/minigames/circus/settings/config.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>
#include <string_view>
#include <vector>

using ce::core::InputFrame;
using ce::core::InputTracker;
using ce::core::KvDocument;
using ce::core::MinigameOutcome;
using ce::core::MinigameSetup;
using ce::core::parse_kv;
using ce::core::PlayStyle;
using ce::core::circus::Campaign;
using ce::core::circus::CircusMinigame;
using ce::core::circus::Event;
using ce::core::circus::EventKind;
using ce::core::circus::GamePhase;
using ce::core::circus::LevelData;

namespace fs = std::filesystem;

namespace
{

// A level without hoops: hold A and the elephant walks to the end.
LevelData short_level(const fs::path&)
{
    LevelData level;
    level.title = "short";
    level.location = "arena";
    level.length = 3.0;
    level.start_x = 0.0;
    return level;
}

Campaign one_level()
{
    Campaign campaign;
    campaign.levels.push_back("short");
    return campaign;
}

void step(CircusMinigame& game, InputTracker& tracker, bool a, double dt)
{
    ce::core::ButtonsDown down{};
    down[static_cast<std::size_t>(ce::core::GameButton::A)] = a;
    const InputFrame input = tracker.next(down);
    game.step({&input, 1}, dt);
}

} // namespace

TEST(Circus, Info)
{
    EXPECT_EQ(ce::core::circus::info.id, "circus");
    EXPECT_FALSE(ce::core::circus::info.title.empty());
    EXPECT_EQ(ce::core::circus::info.style, PlayStyle::Turns);
    EXPECT_NE(ce::core::circus::info.check_config, nullptr);
}

TEST(Circus, LevelsLiveInTheirOwnFolder)
{
    EXPECT_EQ(ce::core::circus::campaign_path("assets"),
              fs::path{"assets/minigames/circus/levels/campaign.txt"});
}

TEST(Circus, OnePlayerAtATime)
{
    EXPECT_NO_THROW((CircusMinigame{one_level(), short_level, KvDocument{}, MinigameSetup{1}}));
    EXPECT_THROW((CircusMinigame{one_level(), short_level, KvDocument{}, MinigameSetup{2}}),
                 std::invalid_argument);
}

TEST(Circus, OverAfterTheLastLevel)
{
    CircusMinigame game{one_level(), short_level, KvDocument{}, MinigameSetup{1, 7}};
    EXPECT_EQ(game.game().record(), 7);
    InputTracker tracker;
    for (int i = 0; i < 400 && game.game().phase() == GamePhase::Playing; ++i)
    {
        step(game, tracker, true, 0.01);
    }
    ASSERT_EQ(game.game().phase(), GamePhase::CampaignComplete);
    EXPECT_EQ(game.outcome(), MinigameOutcome::Playing);
    step(game, tracker, false, 0.01);
    step(game, tracker, true, 0.01);
    EXPECT_EQ(game.outcome(), MinigameOutcome::Won);
    EXPECT_EQ(game.points(0), 0);
}

TEST(Circus, KeepsEventsUntilTaken)
{
    CircusMinigame game{one_level(), short_level, KvDocument{}};
    InputTracker tracker;
    for (int i = 0; i < 400 && game.game().phase() == GamePhase::Playing; ++i)
    {
        step(game, tracker, true, 0.01);
    }
    const std::vector<Event> events = game.take_events();
    ASSERT_FALSE(events.empty());
    EXPECT_EQ(events.back().kind, EventKind::LevelFinished);
    EXPECT_TRUE(game.take_events().empty());
}

TEST(Circus, NumbersFromTheGameConfig)
{
    const KvDocument config = parse_kv("[circus.elephant]\nwalk_speed = 5\n");
    CircusMinigame game{one_level(), short_level, config};
    EXPECT_DOUBLE_EQ(game.game().world().tunables().walk_speed, 5.0);

    game.apply_config(parse_kv("[circus.elephant]\nwalk_speed = 2\n[pad]\nlayout = xbox\n"));
    EXPECT_DOUBLE_EQ(game.game().world().tunables().walk_speed, 2.0);
}

TEST(Circus, CheckConfigLooksOnlyAtCircusSections)
{
    const auto check = ce::core::circus::info.check_config;
    EXPECT_NO_THROW(check(parse_kv("[pad]\nlayout = xbox\n[ball.speed]\nfast = 1\n")));
    EXPECT_THROW(check(parse_kv("[circus.elefant]\nwalk_speed = 1\n")), ce::core::KvError);
    EXPECT_THROW(check(parse_kv("[circus.game]\nwalk_speed = 1\n")), ce::core::KvError);
}
