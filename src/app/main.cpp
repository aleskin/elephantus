#include "app/minigames.hpp"
#include "app/screen_shots.hpp"
#include "app/script_run.hpp"
#include "core/game_info.hpp"
#include "core/input/controls.hpp"
#include "core/input/controls_menu.hpp"
#include "core/input/input_frame.hpp"
#include "core/party/flow.hpp"
#include "core/settings/config.hpp"
#include "core/settings/user_files.hpp"
#include "core/time/fixed_step.hpp"
#include "input/gamepad.hpp"
#include "input/keyboard.hpp"
#include "input/menu.hpp"
#include "render/screens/screens.hpp"
#include "render/text.hpp"
#include "sound/sounds.hpp"

#include <raylib.h>

#include <cstddef>
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
// that new numbers are in use.
void draw_config_note(const ce::core::ConfigWatcher& config, double since_reload)
{
    const int font = 20;
    const int y = window_height - font - 10;
    if (!config.error().empty())
    {
        const std::string text = "config: " + config.error();
        DrawRectangle(0, y - 6, window_width, font + 12, Fade(BLACK, 0.7F));
        ce::render::draw_text(text, 10, y, font, RED);
    }
    else if (since_reload < reloaded_note_time)
    {
        ce::render::draw_text("config reloaded", 10, y, font, DARKGREEN);
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

// The controls the player saved in "Settings" -> "Controls"; nullopt means the defaults.
// A broken file is reported and the defaults are used, the file stays as it is.
std::optional<ce::core::Controls> load_saved_controls(ce::core::PadLayout layout)
{
    const fs::path dir = ce::core::user_config_dir();
    if (dir.empty())
    {
        return std::nullopt;
    }
    try
    {
        return ce::core::load_controls(dir / "controls.txt", layout);
    }
    catch (const std::exception& error)
    {
        std::cerr << "circus_elephant: controls: " << error.what() << "; using the defaults\n";
        return std::nullopt;
    }
}

// Keeps the controls changed in "Settings" -> "Controls" for the next runs.
void save_controls(const ce::core::Controls& controls)
{
    const fs::path dir = ce::core::user_config_dir();
    if (dir.empty())
    {
        return;
    }
    try
    {
        ce::core::write_text_file(dir / "controls.txt", ce::core::format_controls(controls));
    }
    catch (const std::exception& error)
    {
        std::cerr << "circus_elephant: controls: " << error.what() << "\n";
    }
}

// While the controls menu waits, the next key or gamepad button goes to it and nothing to
// the other menus. Returns true when the frame was used up like that.
bool wait_for_binding(ce::core::ControlsMenu& menu, ce::sound::Sounds& sounds)
{
    const std::optional<int> key = ce::input::pressed_key();
    if (!menu.waiting())
    {
        return false;
    }
    const std::optional<ce::core::PadButton> pad = ce::input::pressed_pad_button();
    if (key)
    {
        menu.assign_key(*key);
    }
    else if (pad)
    {
        menu.assign_pad(*pad);
    }
    else if (ce::input::leave_pressed())
    {
        menu.cancel();
    }
    if (!menu.waiting())
    {
        sounds.play(ce::sound::Cue::MenuSelect);
    }
    return true;
}

void play_menu_sound(ce::sound::Sounds& sounds, const ce::core::MenuInput& input,
                     bool screen_changed, bool cursor_moved)
{
    if (screen_changed)
    {
        sounds.play(input.back ? ce::sound::Cue::MenuBack : ce::sound::Cue::MenuSelect);
    }
    else if (cursor_moved)
    {
        sounds.play(ce::sound::Cue::MenuMove);
    }
}

// The whole game: title, menus and the chosen mini-game, until "Quit" or the window closes.
void play(const fs::path& assets, ce::core::ConfigWatcher& config)
{
    using ce::core::Screen;
    ce::sound::Sounds sounds;
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
    std::optional<ce::core::Controls> saved_controls =
        load_saved_controls(config.config().pad_layout);
    std::optional<ce::core::ControlsMenu> controls_menu;
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

        const ce::core::Controls controls =
            saved_controls.value_or(ce::core::default_controls(config.config().pad_layout));
        ce::core::ButtonsDown down = ce::input::read_keyboard(controls);
        ce::input::read_gamepads(controls, down);
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
                sounds.play(ce::sound::Cue::MenuBack);
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
                for (const ce::sound::Cue cue : running->take_cues())
                {
                    sounds.play(cue);
                }
                if (running->game().outcome() != ce::core::MinigameOutcome::Playing)
                {
                    // TODO(CEL-00037): the result screen comes in between.
                    running.reset();
                    flow.minigame_over();
                }
            }
        }
        else if (flow.screen() == Screen::Controls && controls_menu)
        {
            if (!wait_for_binding(*controls_menu, sounds))
            {
                const std::size_t row = controls_menu->row();
                const ce::core::ControlsColumn column = controls_menu->column();
                controls_menu->input(menu_input);
                flow.input(menu_input);
                if (flow.screen() != Screen::Controls)
                {
                    sounds.play(ce::sound::Cue::MenuBack);
                    controls_menu.reset();
                }
                else if (menu_input.confirm)
                {
                    sounds.play(ce::sound::Cue::MenuSelect);
                }
                else if (row != controls_menu->row() || column != controls_menu->column())
                {
                    sounds.play(ce::sound::Cue::MenuMove);
                }
            }
            if (controls_menu && controls_menu->take_changed())
            {
                saved_controls = controls_menu->controls();
                save_controls(*saved_controls);
            }
        }
        else
        {
            const Screen before = flow.screen();
            const std::size_t cursor_before = flow.cursor();
            flow.input(menu_input);
            play_menu_sound(sounds, menu_input, before != flow.screen(),
                            cursor_before != flow.cursor());
            title_time = flow.screen() == Screen::Title && before == Screen::Title
                             ? title_time + frame
                             : 0.0;
        }

        if (flow.screen() == Screen::Controls && !controls_menu)
        {
            controls_menu.emplace(controls, config.config().pad_layout);
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
        case Screen::Controls:
            ce::render::draw_controls(*controls_menu, config.config().pad_layout);
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
            const ce::render::GameFont font{assets};
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
