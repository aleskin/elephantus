#pragma once

#include "core/level/level.hpp"

#include <string_view>

namespace ce::render
{

// The look of a place: floor, walls, background. Drawn inside BeginMode3D.
// A new place = a new file in render/locations/ + one line in the registry.
struct LocationView
{
    std::string_view name;
    void (*draw)(const core::LevelData& level);
};

// nullptr if there is no view for this location name.
const LocationView* find_location(std::string_view name);

void draw_arena(const core::LevelData& level);

} // namespace ce::render
