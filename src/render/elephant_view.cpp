#include "render/elephant_view.hpp"

#include <raylib.h>
#include <rlgl.h>

namespace ce::render
{

namespace
{

constexpr float body_depth = 1.0f;
constexpr Color body_color = LIGHTGRAY;
constexpr Color edge_color = DARKGRAY;
constexpr Color fallen_color{200, 120, 120, 255};

// TODO(CEL-00018): grey blocks until the elephant gets a shape; one block per state so
// the state is visible in the screenshots.
constexpr float upright_width = 0.9f;
constexpr float upright_height = 1.7f;
constexpr float head_size = 0.5f;
constexpr float ball_size = 0.9f;

void block(Vector3 centre, float width, float height, Color color)
{
    DrawCube(centre, width, height, body_depth, color);
    DrawCubeWires(centre, width, height, body_depth, edge_color);
}

void draw_upright(float x)
{
    block(Vector3{x, upright_height / 2.0f, 0.0f}, upright_width, upright_height, body_color);
    // A small block in front marks where the elephant looks.
    block(Vector3{x + upright_width / 2.0f, upright_height - head_size / 2.0f, 0.0f}, head_size,
          head_size, body_color);
}

void draw_salto(const core::Elephant& elephant)
{
    const float x = static_cast<float>(elephant.x);
    const float cy = static_cast<float>(elephant.centre_y());
    // TODO(CEL-00010): forward turn for now: clockwise seen from the camera while going +x.
    const float angle = -360.0f * static_cast<float>(elephant.salto);

    rlPushMatrix();
    rlTranslatef(x, cy, 0.0f);
    rlRotatef(angle, 0.0f, 0.0f, 1.0f);
    block(Vector3{0.0f, 0.0f, 0.0f}, ball_size, ball_size, body_color);
    block(Vector3{ball_size / 2.0f, ball_size / 2.0f - head_size / 2.0f, 0.0f}, head_size / 2.0f,
          head_size, body_color);
    rlPopMatrix();
}

} // namespace

void draw_elephant(const core::Elephant& elephant)
{
    const float x = static_cast<float>(elephant.x);
    const float length = static_cast<float>(core::Elephant::length);
    const float height = static_cast<float>(core::Elephant::height);

    switch (elephant.state)
    {
    case core::ElephantState::Standing:
        block(Vector3{x, height / 2.0f, 0.0f}, length, height, body_color);
        break;
    case core::ElephantState::Walking:
        draw_upright(x);
        break;
    case core::ElephantState::Jumping:
        draw_salto(elephant);
        break;
    case core::ElephantState::Fallen:
        block(Vector3{x, height / 4.0f, 0.0f}, length, height / 2.0f, fallen_color);
        break;
    }
}

} // namespace ce::render
