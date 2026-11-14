#include "render/locations/location.hpp"

#include <raylib.h>

namespace ce::render
{

namespace
{

// Grey prototype: the floor strip of the ring, a low barrier behind the path and a back wall.
constexpr float margin = 8.0f;
constexpr float floor_front_z = 11.0f;
constexpr float barrier_z = -2.5f;
constexpr float barrier_height = 0.5f;
constexpr float barrier_thickness = 0.3f;
constexpr float wall_z = -8.0f;
constexpr float wall_height = 10.0f;
constexpr float marker_step = 5.0f;

} // namespace

void draw_arena(const core::LevelData& level)
{
    const float length = static_cast<float>(level.length);
    const float total = length + 2.0f * margin;
    const float centre_x = length / 2.0f;

    DrawPlane(Vector3{centre_x, 0.0f, (floor_front_z + barrier_z) / 2.0f},
              Vector2{total, floor_front_z - barrier_z}, Color{150, 150, 150, 255});
    DrawPlane(Vector3{centre_x, -0.01f, (wall_z + barrier_z) / 2.0f},
              Vector2{total, barrier_z - wall_z}, Color{110, 110, 110, 255});

    DrawCube(Vector3{centre_x, barrier_height / 2.0f, barrier_z}, total, barrier_height,
             barrier_thickness, Color{180, 180, 180, 255});
    DrawCubeWires(Vector3{centre_x, barrier_height / 2.0f, barrier_z}, total, barrier_height,
                  barrier_thickness, DARKGRAY);

    DrawCube(Vector3{centre_x, wall_height / 2.0f, wall_z}, total, wall_height, 0.2f,
             Color{80, 80, 90, 255});

    for (float x = 0.0f; x <= length + 0.01f; x += marker_step)
    {
        DrawCube(Vector3{x, 0.005f, 1.0f}, 0.05f, 0.01f, 0.6f, DARKGRAY);
    }
    DrawCube(Vector3{length, 0.01f, 0.0f}, 0.15f, 0.02f, 4.0f, WHITE);
}

} // namespace ce::render
