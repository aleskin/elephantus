#include "render/minigames/circus/object_views.hpp"

#include <raylib.h>

#include <cmath>

namespace ce::render::circus
{

namespace
{

constexpr int ring_segments = 48;
constexpr float ring_thickness = 0.06f;
constexpr float stand_radius = 0.04f;
constexpr Color hoop_color{200, 200, 200, 255};
constexpr Color stand_color{120, 120, 120, 255};
constexpr Color podium_color{200, 60, 70, 255};
constexpr Color podium_top_color{250, 204, 80, 255};
constexpr float podium_depth = 2.0f;
constexpr float podium_top = 0.06f;

template <typename... Ts> struct Overloaded : Ts...
{
    using Ts::operator()...;
};

} // namespace

void draw_object(const core::circus::LevelObject& object)
{
    std::visit(
        Overloaded{
            [](const core::circus::Hoop& hoop)
            {
                draw_hoop(hoop);
            },
            [](const core::circus::Podium& podium)
            {
                draw_podium(podium);
            },
        },
        object);
}

void draw_hoop(const core::circus::Hoop& hoop)
{
    // The ring stands across the path (in the y-z plane) so the elephant passes through it.
    const float x = static_cast<float>(hoop.x);
    const float cy = static_cast<float>(hoop.height);
    const float r = static_cast<float>(hoop.radius);

    for (int i = 0; i < ring_segments; ++i)
    {
        const float a0 = 2.0f * PI * static_cast<float>(i) / ring_segments;
        const float a1 = 2.0f * PI * static_cast<float>(i + 1) / ring_segments;
        const Vector3 p0{x, cy + r * std::sin(a0), r * std::cos(a0)};
        const Vector3 p1{x, cy + r * std::sin(a1), r * std::cos(a1)};
        DrawCylinderEx(p0, p1, ring_thickness, ring_thickness, 6, hoop_color);
        DrawSphere(p0, ring_thickness, hoop_color);
    }

    const float stand_z = -(r + 0.3f);
    DrawCylinderEx(Vector3{x, 0.0f, stand_z}, Vector3{x, cy, stand_z}, stand_radius, stand_radius,
                   8, stand_color);
    DrawCylinderEx(Vector3{x, cy, stand_z}, Vector3{x, cy, -r}, stand_radius, stand_radius, 8,
                   stand_color);
    DrawCylinder(Vector3{x, 0.0f, stand_z}, 0.25f, 0.25f, 0.05f, 12, stand_color);
}

void draw_podium(const core::circus::Podium& podium)
{
    // A red circus box with a yellow top, the finish of the level.
    const float x = static_cast<float>(podium.x);
    const float w = static_cast<float>(podium.width);
    const float h = static_cast<float>(podium.height);
    DrawCube(Vector3{x, (h - podium_top) / 2.0f, 0.0f}, w, h - podium_top, podium_depth,
             podium_color);
    DrawCubeWires(Vector3{x, (h - podium_top) / 2.0f, 0.0f}, w, h - podium_top, podium_depth,
                  podium_top_color);
    DrawCube(Vector3{x, h - podium_top / 2.0f, 0.0f}, w + 0.1f, podium_top, podium_depth + 0.1f,
             podium_top_color);
}

} // namespace ce::render::circus
