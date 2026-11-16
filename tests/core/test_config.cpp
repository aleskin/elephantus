#include "core/game/game.hpp"
#include "core/input/combo.hpp"
#include "core/input/input_frame.hpp"
#include "core/settings/config.hpp"
#include "core/text/kv_file.hpp"
#include "core/world/world.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

using ce::core::ButtonsDown;
using ce::core::Campaign;
using ce::core::Combo;
using ce::core::ConfigWatcher;
using ce::core::Game;
using ce::core::GameButton;
using ce::core::GameConfig;
using ce::core::InputFrame;
using ce::core::InputTracker;
using ce::core::KvError;
using ce::core::LevelData;
using ce::core::load_config;
using ce::core::PadLayout;
using ce::core::parse_config;
using ce::core::parse_kv;
using ce::core::Tunables;
using ce::core::World;

namespace fs = std::filesystem;

namespace
{

Tunables parse(std::string_view text)
{
    return parse_config(parse_kv(text)).tunables;
}

int error_line(std::string_view text)
{
    try
    {
        parse(text);
    }
    catch (const KvError& error)
    {
        return error.line();
    }
    return -1;
}

// A config file in a temporary folder that the test rewrites.
class TempConfig
{
public:
    explicit TempConfig(const std::string& name)
        : path_(fs::temp_directory_path() / ("ce_config_" + name + ".txt"))
    {
        fs::remove(path_);
    }
    ~TempConfig()
    {
        std::error_code ignored;
        fs::remove(path_, ignored);
    }
    TempConfig(const TempConfig&) = delete;
    TempConfig& operator=(const TempConfig&) = delete;

    const fs::path& path() const
    {
        return path_;
    }

    // Writes the file and moves its time forward, so the change is seen even when the
    // file system keeps time in whole seconds.
    void write(std::string_view text)
    {
        {
            std::ofstream out{path_};
            out << text;
        }
        stamp_ += std::chrono::seconds{1};
        fs::last_write_time(path_, stamp_);
    }

private:
    fs::path path_;
    fs::file_time_type stamp_ = fs::file_time_type::clock::now();
};

LevelData empty_level()
{
    LevelData level;
    level.title = "Test";
    level.location = "arena";
    level.length = 100.0;
    level.start_x = 0.0;
    return level;
}

ButtonsDown down(std::string_view buttons)
{
    ButtonsDown result{};
    for (const char c : buttons)
    {
        result[static_cast<std::size_t>(*ce::core::button_from_name(std::string(1, c)))] = true;
    }
    return result;
}

} // namespace

TEST(Config, EmptyFileKeepsTheNumbersFromTheCode)
{
    const Tunables tunables = parse("# nothing here\n");
    const Tunables defaults;
    EXPECT_DOUBLE_EQ(tunables.walk_speed, defaults.walk_speed);
    EXPECT_DOUBLE_EQ(tunables.jump_height, defaults.jump_height);
    EXPECT_DOUBLE_EQ(tunables.gravity, defaults.gravity);
    EXPECT_DOUBLE_EQ(tunables.fall_grace, defaults.fall_grace);
    EXPECT_DOUBLE_EQ(tunables.salto_window, defaults.salto_window);
    EXPECT_DOUBLE_EQ(tunables.restart_delay, defaults.restart_delay);
}

TEST(Config, ReadsEveryNumber)
{
    const Tunables tunables = parse("[elephant]\n"
                                    "walk_speed = 4,5\n"
                                    "jump_height = 2\n"
                                    "gravity = 15\n"
                                    "fall_grace = 0.5\n"
                                    "[buttons]\n"
                                    "salto_window = 0.6\n"
                                    "[game]\n"
                                    "restart_delay = 2\n");
    EXPECT_DOUBLE_EQ(tunables.walk_speed, 4.5);
    EXPECT_DOUBLE_EQ(tunables.jump_height, 2.0);
    EXPECT_DOUBLE_EQ(tunables.gravity, 15.0);
    EXPECT_DOUBLE_EQ(tunables.fall_grace, 0.5);
    EXPECT_DOUBLE_EQ(tunables.salto_window, 0.6);
    EXPECT_DOUBLE_EQ(tunables.restart_delay, 2.0);
}

TEST(Config, MissingKeyKeepsTheNumberFromTheCode)
{
    const Tunables tunables = parse("[elephant]\nwalk_speed = 5\n");
    EXPECT_DOUBLE_EQ(tunables.walk_speed, 5.0);
    EXPECT_DOUBLE_EQ(tunables.gravity, Tunables{}.gravity);
}

TEST(Config, RejectsTyposAndBadNumbers)
{
    EXPECT_EQ(error_line("[elephant]\nwalk_sped = 5\n"), 2);
    EXPECT_EQ(error_line("[elefant]\nwalk_speed = 5\n"), 1);
    EXPECT_EQ(error_line("[game]\nwalk_speed = 5\n"), 2);
    EXPECT_EQ(error_line("walk_speed = 5\n"), 1);
    EXPECT_EQ(error_line("[elephant]\n\nwalk_speed = 0\n"), 3);
    EXPECT_EQ(error_line("[elephant]\nwalk_speed = -1\n"), 2);
    EXPECT_EQ(error_line("[elephant]\nwalk_speed = fast\n"), 2);
}

TEST(Config, PadLayout)
{
    EXPECT_EQ(parse_config(parse_kv("")).pad_layout, PadLayout::Nintendo);
    EXPECT_EQ(parse_config(parse_kv("[pad]\nlayout = xbox\n")).pad_layout, PadLayout::Xbox);
    EXPECT_EQ(parse_config(parse_kv("[pad]\nlayout = nintendo\n")).pad_layout, PadLayout::Nintendo);
    EXPECT_EQ(error_line("[pad]\nlayout = playstation\n"), 2);
    EXPECT_EQ(error_line("[pad]\nlayot = xbox\n"), 2);
}

TEST(Config, ErrorNamesTheFile)
{
    TempConfig file{"error_name"};
    file.write("[elephant]\ngravity = 0\n");
    try
    {
        load_config(file.path());
        FAIL() << "expected an error";
    }
    catch (const KvError& error)
    {
        EXPECT_EQ(error.file(), file.path().filename().string());
        EXPECT_EQ(error.line(), 2);
    }
}

TEST(Config, GameConfigInAssetsLoads)
{
    EXPECT_NO_THROW(load_config(fs::path{CE_ASSETS_DIR} / "config.txt"));
}

TEST(ConfigWatcher, LoadsTheFileAtOnce)
{
    TempConfig file{"at_once"};
    file.write("[elephant]\nwalk_speed = 5\n");
    const ConfigWatcher watcher{file.path()};
    EXPECT_TRUE(watcher.error().empty());
    EXPECT_DOUBLE_EQ(watcher.config().tunables.walk_speed, 5.0);
}

TEST(ConfigWatcher, MissingFileKeepsTheCodeNumbers)
{
    TempConfig file{"missing"};
    const ConfigWatcher watcher{file.path()};
    EXPECT_FALSE(watcher.error().empty());
    EXPECT_DOUBLE_EQ(watcher.config().tunables.walk_speed, Tunables{}.walk_speed);
}

TEST(ConfigWatcher, PicksUpAChangeOncePerInterval)
{
    TempConfig file{"interval"};
    file.write("[elephant]\nwalk_speed = 5\n");
    ConfigWatcher watcher{file.path(), 1.0};

    file.write("[elephant]\nwalk_speed = 6\n");
    EXPECT_FALSE(watcher.poll(0.5));
    EXPECT_DOUBLE_EQ(watcher.config().tunables.walk_speed, 5.0);
    EXPECT_TRUE(watcher.poll(0.5));
    EXPECT_DOUBLE_EQ(watcher.config().tunables.walk_speed, 6.0);

    EXPECT_FALSE(watcher.poll(1.0));
}

TEST(ConfigWatcher, BrokenFileKeepsTheLastGoodNumbers)
{
    TempConfig file{"broken"};
    file.write("[elephant]\nwalk_speed = 5\n");
    ConfigWatcher watcher{file.path()};

    file.write("[elephant]\nwalk_speed = zero\n");
    EXPECT_FALSE(watcher.check());
    EXPECT_DOUBLE_EQ(watcher.config().tunables.walk_speed, 5.0);
    EXPECT_NE(watcher.error().find("line 2"), std::string::npos) << watcher.error();

    file.write("[elephant]\nwalk_speed = 7\n");
    EXPECT_TRUE(watcher.check());
    EXPECT_TRUE(watcher.error().empty());
    EXPECT_DOUBLE_EQ(watcher.config().tunables.walk_speed, 7.0);
}

TEST(ConfigWatcher, FileThatAppearsLaterIsLoaded)
{
    TempConfig file{"later"};
    ConfigWatcher watcher{file.path()};
    file.write("[game]\nrestart_delay = 3\n");
    EXPECT_TRUE(watcher.check());
    EXPECT_TRUE(watcher.error().empty());
    EXPECT_DOUBLE_EQ(watcher.config().tunables.restart_delay, 3.0);
}

TEST(Combo, NewWindowKeepsTheStartedSequence)
{
    Combo combo{{GameButton::X, GameButton::B}, 0.2};
    InputTracker tracker;
    combo.step(tracker.next(down("x")), 0.01);
    combo.set_window(1.0);
    tracker.next(down(""));
    EXPECT_FALSE(combo.step(tracker.next(down("")), 0.5));
    EXPECT_TRUE(combo.step(tracker.next(down("b")), 0.01));
}

TEST(Config, NewNumbersApplyToTheWalkingElephant)
{
    World world{empty_level()};
    InputTracker tracker;
    world.step(tracker.next(down("a")), 0.1);
    world.step(tracker.next(down("a")), 1.0);
    const double before = world.elephant().x;

    Tunables faster;
    faster.walk_speed = 6.0;
    world.set_tunables(faster);
    world.step(tracker.next(down("a")), 1.0);
    EXPECT_NEAR(world.elephant().x - before, 6.0, 1e-9);
}

TEST(Config, GameKeepsNewNumbersAfterARestart)
{
    Campaign campaign;
    campaign.levels.push_back("test");
    Game game{campaign, [](const fs::path&)
              {
                  return empty_level();
              }};

    Tunables quick;
    quick.restart_delay = 0.2;
    quick.walk_speed = 5.0;
    game.set_tunables(quick);
    EXPECT_DOUBLE_EQ(game.world().tunables().walk_speed, 5.0);

    InputTracker tracker;
    game.step(tracker.next(down("a")), 0.01);
    for (int i = 0; i < 100 && game.phase() == ce::core::GamePhase::Playing; ++i)
    {
        game.step(tracker.next(down("")), 0.01);
    }
    ASSERT_EQ(game.phase(), ce::core::GamePhase::LevelFailed);
    game.step(tracker.next(down("")), 0.25);
    EXPECT_EQ(game.phase(), ce::core::GamePhase::Playing);
    EXPECT_DOUBLE_EQ(game.world().tunables().walk_speed, 5.0);
}
