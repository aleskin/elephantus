#include "render/elephant_view.hpp"

#include <raylib.h>

namespace ce::render
{

namespace
{

constexpr float body_depth = 1.0f;

} // namespace

void draw_elephant(const core::Elephant& elephant)
{
    const float length = static_cast<float>(core::Elephant::length);
    const float height = static_cast<float>(core::Elephant::height);
    const Vector3 centre{static_cast<float>(elephant.x),
                         static_cast<float>(elephant.y) + height / 2.0f, 0.0f};
    DrawCube(centre, length, height, body_depth, LIGHTGRAY);
    DrawCubeWires(centre, length, height, body_depth, DARKGRAY);
}

} // namespace ce::render
