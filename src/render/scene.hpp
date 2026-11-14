#pragma once

#include "core/world/world.hpp"

namespace ce::render
{

// The whole frame for a world: camera, place, objects, elephant.
// The place must have a view (check with find_location() when the level is loaded).
void draw_scene(const core::World& world);

} // namespace ce::render
