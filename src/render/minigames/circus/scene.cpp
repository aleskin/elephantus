#include "render/minigames/circus/scene.hpp"

#include "render/camera.hpp"
#include "render/minigames/circus/elephant_view.hpp"
#include "render/minigames/circus/locations/location.hpp"
#include "render/minigames/circus/object_views.hpp"

#include <raylib.h>

namespace ce::render::circus
{

namespace
{

// The camera looks a little ahead of the elephant, so the next hoop comes into view early.
constexpr float look_ahead = 6.0f;
constexpr Color background{40, 40, 48, 255};

} // namespace

void draw_scene(const core::circus::World& world)
{
    const core::circus::LevelData& level = world.level();
    const Camera3D camera = side_camera(static_cast<float>(world.elephant().x) + look_ahead);

    ClearBackground(background);
    BeginMode3D(camera);
    if (const LocationView* location = find_location(level.location))
    {
        location->draw(level);
    }
    for (const core::circus::LevelObject& object : level.objects)
    {
        draw_object(object);
    }
    draw_elephant(world.elephant());
    EndMode3D();
}

} // namespace ce::render::circus
