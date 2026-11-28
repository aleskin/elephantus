#pragma once

#include "core/minigames/circus/objects/level_object.hpp"

namespace ce::render::circus
{

// Draws one level object inside BeginMode3D. One function per kind of object.
void draw_object(const core::circus::LevelObject& object);

void draw_hoop(const core::circus::Hoop& hoop);
void draw_podium(const core::circus::Podium& podium);

} // namespace ce::render::circus
