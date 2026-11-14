#include "core/game_info.hpp"
#include "core/level/level.hpp"
#include "render/camera.hpp"
#include "render/locations/location.hpp"
#include "render/object_views.hpp"

#include <raylib.h>

#include <filesystem>
#include <iostream>
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

const ce::render::LocationView& location_for(const ce::core::LevelData& level)
{
    const ce::render::LocationView* view = ce::render::find_location(level.location);
    if (view == nullptr)
    {
        throw std::runtime_error("no view for location '" + level.location + "'");
    }
    return *view;
}

} // namespace

int main(int argc, char** argv)
{
    constexpr int window_width = 1280;
    constexpr int window_height = 720;

    ce::core::LevelData level;
    try
    {
        const fs::path assets = find_assets(argc, argv);
        const ce::core::Campaign campaign =
            ce::core::load_campaign(assets / "levels" / "campaign.txt");
        level = ce::core::load_level(campaign.levels.front());
        location_for(level);
    }
    catch (const std::exception& error)
    {
        std::cerr << "circus_elephant: " << error.what() << '\n';
        return 1;
    }
    const ce::render::LocationView& location = location_for(level);

    const std::string title{ce::core::game_title()};
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(window_width, window_height, title.c_str());
    SetTargetFPS(60);

    // Until the world exists (CEL-00026): a still camera and a grey block at the start.
    const float start_x = static_cast<float>(level.start_x);
    const Camera3D camera = ce::render::side_camera(start_x + 6.0f);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(Color{40, 40, 48, 255});
        BeginMode3D(camera);
        location.draw(level);
        for (const ce::core::LevelObject& object : level.objects)
        {
            ce::render::draw_object(object);
        }
        DrawCube(Vector3{start_x, 0.6f, 0.0f}, 1.6f, 1.2f, 1.0f, LIGHTGRAY);
        DrawCubeWires(Vector3{start_x, 0.6f, 0.0f}, 1.6f, 1.2f, 1.0f, DARKGRAY);
        EndMode3D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
