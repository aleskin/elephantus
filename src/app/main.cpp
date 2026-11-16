#include "app/minigames.hpp"
#include "app/script_run.hpp"
#include "core/game_info.hpp"
#include "core/input/input_frame.hpp"
#include "core/settings/config.hpp"
#include "core/time/fixed_step.hpp"
#include "input/gamepad.hpp"
#include "input/keyboard.hpp"

#include <raylib.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{

namespace fs = std::filesystem;

constexpr int window_width = 1280;
constexpr int window_height = 720;

// circus_elephant [--assets DIR] [--minigame ID] [--script FILE [--shots DIR]]
struct Options
{
    std::optional<fs::path> assets;
    // TODO(CEL-00032): until there is a menu, the game starts this mini-game right away.
    std::string minigame = "circus";
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
        else if (name == "--minigame")
        {
            options.minigame = value.string();
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

// How long "config reloaded" stays on the screen, seconds.
constexpr double reloaded_note_time = 2.0;

// A line at the bottom of the screen about the game config: its error, or a short note
// that new numbers are in use. raylib's built-in font has no Cyrillic, so English.
void draw_config_note(const ce::core::ConfigWatcher& config, double since_reload)
{
    const int font = 20;
    const int y = window_height - font - 10;
    if (!config.error().empty())
    {
        const std::string text = "config: " + config.error();
        DrawRectangle(0, y - 6, window_width, font + 12, Fade(BLACK, 0.7F));
        DrawText(text.c_str(), 10, y, font, RED);
    }
    else if (since_reload < reloaded_note_time)
    {
        DrawText("config reloaded", 10, y, font, DARKGREEN);
    }
}

void play(ce::render::MinigamePlay& minigame, ce::core::ConfigWatcher& config)
{
    ce::core::FixedStep clock;
    ce::core::InputTracker tracker;
    double since_reload = reloaded_note_time;
    std::optional<std::string> pads;
    while (!WindowShouldClose() && minigame.game().outcome() == ce::core::MinigameOutcome::Playing)
    {
        const double frame = GetFrameTime();
        since_reload += frame;
        if (config.poll(frame))
        {
            minigame.game().apply_config(config.config().document);
            since_reload = 0.0;
        }
        const int steps = clock.advance(frame);
        const std::string now_pads = ce::input::gamepad_names();
        if (!pads || now_pads != *pads)
        {
            pads = now_pads;
            std::cout << "circus_elephant: gamepads: " << (now_pads.empty() ? "none" : now_pads)
                      << std::endl;
        }
        ce::core::ButtonsDown down = ce::input::read_keyboard();
        ce::input::read_gamepads(config.config().pad_layout, down);
        for (int i = 0; i < steps; ++i)
        {
            const ce::core::InputFrame input = tracker.next(down);
            minigame.game().step({&input, 1}, clock.dt());
        }

        BeginDrawing();
        minigame.draw();
        draw_config_note(config, since_reload);
        EndDrawing();
    }
}

} // namespace

int main(int argc, char** argv)
{
    try
    {
        const Options options = parse_options(argc, argv);
        const fs::path assets = find_assets(options);
        const ce::app::MinigameModule* module = ce::app::find_minigame(options.minigame);
        if (module == nullptr)
        {
            throw std::invalid_argument("unknown mini-game " + options.minigame);
        }
        // Scripts play with the numbers from the code, so the shots do not depend on the
        // game config; the game itself watches assets/config.txt.
        std::optional<ce::core::ConfigWatcher> config;
        if (!options.script)
        {
            config.emplace(assets / "config.txt", ce::app::minigame_infos());
        }
        // TODO(CEL-00046): one player until players can join with their own controllers.
        const std::unique_ptr<ce::render::MinigamePlay> minigame =
            module->start(assets, config ? config->config().document : ce::core::KvDocument{},
                          ce::core::MinigameSetup{});

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
                ce::app::run_script(*minigame, *options.script, options.shots);
            }
            else
            {
                play(*minigame, *config);
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
