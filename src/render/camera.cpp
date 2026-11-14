#include "render/camera.hpp"

namespace ce::render
{

namespace
{

constexpr float distance = 12.0f;
constexpr float eye_height = 2.5f;
constexpr float look_height = 1.2f;
constexpr float field_of_view = 45.0f;

} // namespace

Camera3D side_camera(float focus_x)
{
    Camera3D camera{};
    camera.position = Vector3{focus_x, eye_height, distance};
    camera.target = Vector3{focus_x, look_height, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = field_of_view;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}

} // namespace ce::render
