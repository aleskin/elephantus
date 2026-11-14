#include "app/script_run.hpp"
#include "core/game/game.hpp"
#include "core/game_info.hpp"
#include "core/input/input_frame.hpp"
#include "core/level/level.hpp"
#include "core/world/fixed_step.hpp"
#include "input/keyboard.hpp"
#include "render/locations/location.hpp"
#include "render/scene.hpp"

#include <raylib.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{

namespace fs = std::filesystem;

constexpr int window_width = 1280;
constexpr int window_height = 720;

// circus_elephant [--assets DIR] [--script FILE [--shots DIR]]
struct Options
{
    std::optional<fs::path> assets;
    std::optional<fs::path> script;
    fs::path shots = "build/shots";
};

Options parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view name{argv[i]};
        if (i + 1 >= argc)
        {
            throw std::invalid_argument("missing value after " + std::string{name});
        }
        const fs::path value{argv[++i]};
        if (name == "--assets")
        {
            options.assets = value;
        }
        else if (name == "--script")
        {
            options.script = value;
        }
        else if (name == "--shots")
        {
            options.shots = value;
        }
        else
        {
            throw std::invalid_argument("unknown option " + std::string{name});
        }
    }
    return options;
}

// --assets DIR, otherwise ./assets, otherwise assets next to the build folder.
fs::path find_assets(const Options& options)
{
    if (options.assets)
    {
        return *options.assets;
    }
    if (fs::is_directory("assets"))
    {
        return "assets";
    }
    return fs::path{GetApplicationDirectory()} / ".." / "assets";
}

// Loads a level and makes sure the game knows how to draw its place.
ce::core::LevelData load_drawable_level(const fs::path& path)
{
    ce::core::LevelData level = ce::core::load_level(path);
    if (ce::render::find_location(level.location) == nullptr)
    {
        throw std::runtime_error(path.filename().string() + ": no view for location '" +
                                 level.location + "'");
    }
    return level;
}

void play(ce::core::Game& game)
{
    ce::core::FixedStep clock;
    ce::core::InputTracker tracker;
    while (!WindowShouldClose() && game.phase() != ce::core::GamePhase::CampaignComplete)
    {
        const int steps = clock.advance(GetFrameTime());
        const ce::core::ButtonsDown down = ce::input::read_keyboard();
        for (int i = 0; i < steps; ++i)
        {
            game.step(tracker.next(down), clock.dt());
        }

        BeginDrawing();
        ce::render::draw_scene(game.world());
        EndDrawing();
    }
}

} // namespace

int main(int argc, char** argv)
{
    try
    {
        const Options options = parse_options(argc, argv);
        ce::core::Game game{
            ce::core::load_campaign(find_assets(options) / "levels" / "campaign.txt"),
            load_drawable_level};

        const std::string title =
            std::string{ce::core::game_title()} + " " + std::string{ce::core::game_version()};
        unsigned int flags = FLAG_MSAA_4X_HINT;
        if (options.script)
        {
            flags |= FLAG_WINDOW_HIDDEN;
        }
        SetConfigFlags(flags);
        SetTraceLogLevel(options.script ? LOG_WARNING : LOG_INFO);
        InitWindow(window_width, window_height, title.c_str());
        if (!IsWindowReady())
        {
            throw std::runtime_error("cannot open a window");
        }
        SetTargetFPS(60);

        try
        {
            if (options.script)
            {
                ce::app::run_script(game, *options.script, options.shots);
            }
            else
            {
                play(game);
            }
        }
        catch (...)
        {
            CloseWindow();
            throw;
        }
        CloseWindow();
    }
    catch (const std::exception& error)
    {
        std::cerr << "circus_elephant: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
