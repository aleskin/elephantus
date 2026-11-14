#include "core/game_info.hpp"

#include <raylib.h>

#include <string>

int main()
{
    constexpr int window_width = 1280;
    constexpr int window_height = 720;

    const std::string title{ce::core::game_title()};
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(window_width, window_height, title.c_str());
    SetTargetFPS(60);

    Camera3D camera{};
    camera.position = Vector3{0.0f, 2.0f, 12.0f};
    camera.target = Vector3{0.0f, 1.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(Color{40, 40, 48, 255});
        BeginMode3D(camera);
        DrawPlane(Vector3{0.0f, 0.0f, 0.0f}, Vector2{40.0f, 8.0f}, GRAY);
        DrawCube(Vector3{0.0f, 0.5f, 0.0f}, 1.0f, 1.0f, 1.0f, LIGHTGRAY);
        DrawCubeWires(Vector3{0.0f, 0.5f, 0.0f}, 1.0f, 1.0f, 1.0f, DARKGRAY);
        EndMode3D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
