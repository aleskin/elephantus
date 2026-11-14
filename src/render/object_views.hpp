#pragma once

#include "core/objects/level_object.hpp"

namespace ce::render
{

// Draws one level object inside BeginMode3D. One function per kind of object.
void draw_object(const core::LevelObject& object);

void draw_hoop(const core::Hoop& hoop);

} // namespace ce::render
