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

// --assets DIR, otherwise ./assets, otherwise assets next to the build folder.
fs::path find_assets(int argc, char** argv)
{
    for (int i = 1; i + 1 < argc; ++i)
    {
        if (std::string_view{argv[i]} == "--assets")
        {
            return argv[i + 1];
        }
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

} // namespace

int main(int argc, char** argv)
{
    constexpr int window_width = 1280;
    constexpr int window_height = 720;

    std::optional<ce::core::Game> game;
    try
    {
        const fs::path assets = find_assets(argc, argv);
        game.emplace(ce::core::load_campaign(assets / "levels" / "campaign.txt"),
                     load_drawable_level);
    }
    catch (const std::exception& error)
    {
        std::cerr << "circus_elephant: " << error.what() << '\n';
        return 1;
    }

    const std::string title{ce::core::game_title()};
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(window_width, window_height, title.c_str());
    SetTargetFPS(60);

    ce::core::FixedStep clock;
    ce::core::InputTracker tracker;
    int exit_code = 0;
    while (!WindowShouldClose() && game->phase() != ce::core::GamePhase::CampaignComplete)
    {
        const int steps = clock.advance(GetFrameTime());
        const ce::core::ButtonsDown down = ce::input::read_keyboard();
        try
        {
            for (int i = 0; i < steps; ++i)
            {
                const ce::core::InputFrame frame = tracker.next(down);
                game->step(frame, clock.dt());
                // TODO(CEL-00012): what happens after a level is Polina's call; for now Jump
                // moves on.
                if (game->phase() == ce::core::GamePhase::LevelComplete &&
                    frame.pressed(ce::core::GameButton::Jump))
                {
                    game->next_level();
                }
            }
        }
        catch (const std::exception& error)
        {
            std::cerr << "circus_elephant: " << error.what() << '\n';
            exit_code = 1;
            break;
        }

        BeginDrawing();
        ce::render::draw_scene(game->world());
        EndDrawing();
    }

    CloseWindow();
    return exit_code;
}
