#include "app/minigames.hpp"
#include "app/screen_shots.hpp"
#include "app/script_run.hpp"
#include "core/game_info.hpp"
#include "core/input/input_frame.hpp"
#include "core/party/flow.hpp"
#include "core/settings/config.hpp"
#include "core/time/fixed_step.hpp"
#include "input/gamepad.hpp"
#include "input/keyboard.hpp"
#include "input/menu.hpp"
#include "render/screens/screens.hpp"

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

// circus_elephant [--assets DIR] [--minigame ID --script FILE [--shots DIR]] [--screens DIR]
// Without --script or --screens: the title, the menus and the mini-games.
struct Options
{
    std::optional<fs::path> assets;
    // The mini-game a script plays.
    std::string minigame = "circus";
    std::optional<fs::path> script;
    fs::path shots = "build/shots";
    // Saves a PNG of every menu screen into DIR and exits.
    std::optional<fs::path> screens;
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
        else if (name == "--screens")
        {
            options.screens = value;
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

void log_gamepads(std::optional<std::string>& pads)
{
    const std::string now_pads = ce::input::gamepad_names();
    if (!pads || now_pads != *pads)
    {
        pads = now_pads;
        std::cout << "circus_elephant: gamepads: " << (now_pads.empty() ? "none" : now_pads)
                  << std::endl;
    }
}

bool any_down(const ce::core::ButtonsDown& down)
{
    for (const bool button : down)
    {
        if (button)
        {
            return true;
        }
    }
    return false;
}

// The whole game: title, menus and the chosen mini-game, until "Quit" or the window closes.
void play(const fs::path& assets, ce::core::ConfigWatcher& config)
{
    using ce::core::Screen;
    ce::core::PartyFlow flow{ce::app::minigame_infos()};
    ce::core::MenuTracker menu;
    std::unique_ptr<ce::render::MinigamePlay> running;
    ce::core::FixedStep clock;
    ce::core::InputTracker tracker;
    // The A that chose the mini-game is still held when it starts: the game gets its
    // buttons only after everything is let go.
    bool wait_for_release = false;
    double title_time = 0.0;
    double since_reload = reloaded_note_time;
    std::optional<std::string> pads;
    while (!WindowShouldClose() && flow.screen() != Screen::Quit)
    {
        const double frame = GetFrameTime();
        since_reload += frame;
        if (config.poll(frame))
        {
            if (running)
            {
                running->game().apply_config(config.config().document);
            }
            since_reload = 0.0;
        }
        log_gamepads(pads);

        ce::core::ButtonsDown down = ce::input::read_keyboard();
        ce::input::read_gamepads(config.config().pad_layout, down);
        const ce::core::MenuInput menu_input =
            menu.next(ce::input::read_menu_buttons(config.config().pad_layout));

        if (flow.screen() == Screen::Minigame)
        {
            if (!running)
            {
                const ce::app::MinigameModule* module = ce::app::find_minigame(flow.chosen().id);
                // TODO(CEL-00046): one player until players can join with their own controllers.
                running =
                    module->start(assets, config.config().document, ce::core::MinigameSetup{});
                clock = ce::core::FixedStep{};
                tracker = ce::core::InputTracker{};
                wait_for_release = true;
            }
            if (ce::input::leave_pressed())
            {
                running.reset();
                flow.minigame_over();
            }
            else
            {
                if (wait_for_release && !any_down(down))
                {
                    wait_for_release = false;
                }
                const ce::core::ButtonsDown game_down =
                    wait_for_release ? ce::core::ButtonsDown{} : down;
                const int steps = clock.advance(frame);
                for (int i = 0; i < steps; ++i)
                {
                    const ce::core::InputFrame input = tracker.next(game_down);
                    running->game().step({&input, 1}, clock.dt());
                }
                if (running->game().outcome() != ce::core::MinigameOutcome::Playing)
                {
                    // TODO(CEL-00037): the result screen comes in between.
                    running.reset();
                    flow.minigame_over();
                }
            }
        }
        else
        {
            const Screen before = flow.screen();
            flow.input(menu_input);
            title_time = flow.screen() == Screen::Title && before == Screen::Title
                             ? title_time + frame
                             : 0.0;
        }

        BeginDrawing();
        switch (flow.screen())
        {
        case Screen::Title:
            ce::render::draw_title(title_time);
            break;
        case Screen::MainMenu:
            ce::render::draw_main_menu(flow);
            break;
        case Screen::MinigameList:
            ce::render::draw_minigame_list(flow);
            break;
        case Screen::Minigame:
            if (running)
            {
                running->draw();
            }
            break;
        case Screen::Quit:
            ClearBackground(BLACK);
            break;
        }
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
        const bool scripted = options.script || options.screens;
        // Scripts play with the numbers from the code, so the shots do not depend on the
        // game config; the game itself watches assets/config.txt.
        std::optional<ce::core::ConfigWatcher> config;
        std::unique_ptr<ce::render::MinigamePlay> minigame;
        if (options.script)
        {
            minigame = module->start(assets, ce::core::KvDocument{}, ce::core::MinigameSetup{});
        }
        else if (!options.screens)
        {
            config.emplace(assets / "config.txt", ce::app::minigame_infos());
        }

        const std::string title =
            std::string{ce::core::game_title()} + " " + std::string{ce::core::game_version()};
        unsigned int flags = FLAG_MSAA_4X_HINT;
        if (scripted)
        {
            flags |= FLAG_WINDOW_HIDDEN;
        }
        SetConfigFlags(flags);
        SetTraceLogLevel(scripted ? LOG_WARNING : LOG_INFO);
        InitWindow(window_width, window_height, title.c_str());
        if (!IsWindowReady())
        {
            throw std::runtime_error("cannot open a window");
        }
        SetTargetFPS(60);
        // Esc goes back in the menus and leaves a mini-game; "Quit" in the menu closes.
        SetExitKey(KEY_NULL);

        try
        {
            if (options.script)
            {
                ce::app::run_script(*minigame, *options.script, options.shots);
            }
            else if (options.screens)
            {
                ce::app::save_screens(ce::app::minigame_infos(), *options.screens);
            }
            else
            {
                play(assets, *config);
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
