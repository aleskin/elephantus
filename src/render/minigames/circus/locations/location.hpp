#pragma once

#include "core/minigames/circus/level/level.hpp"

#include <string_view>

namespace ce::render::circus
{

// The look of a place: floor, walls, background. Drawn inside BeginMode3D.
// A new place = a new file in render/minigames/circus/locations/ + one line in the registry.
struct LocationView
{
    std::string_view name;
    void (*draw)(const core::circus::LevelData& level);
};

// nullptr if there is no view for this location name.
const LocationView* find_location(std::string_view name);

void draw_arena(const core::circus::LevelData& level);

} // namespace ce::render::circus
