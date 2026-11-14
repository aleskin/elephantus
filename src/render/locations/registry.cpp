#include "render/locations/location.hpp"

#include <array>

namespace ce::render
{

namespace
{

constexpr std::array<LocationView, 1> locations = {{
    {"arena", &draw_arena},
}};

} // namespace

const LocationView* find_location(std::string_view name)
{
    for (const LocationView& location : locations)
    {
        if (location.name == name)
        {
            return &location;
        }
    }
    return nullptr;
}

} // namespace ce::render
