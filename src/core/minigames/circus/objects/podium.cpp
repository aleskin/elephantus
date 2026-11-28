#include "core/minigames/circus/objects/podium.hpp"

namespace ce::core::circus
{

double Podium::left() const
{
    return x - width / 2.0;
}

double Podium::right() const
{
    return x + width / 2.0;
}

Podium parse_podium(const KvSection& section)
{
    Podium podium;
    podium.x = section.require_number("x");
    podium.width = section.number("width", Podium::default_width);
    podium.height = section.number("height", Podium::default_height);
    if (podium.width <= 0.0)
    {
        throw KvError(section.find("width")->line, "podium width must be above zero");
    }
    if (podium.height <= 0.0)
    {
        throw KvError(section.find("height")->line, "podium height must be above zero");
    }
    return podium;
}

} // namespace ce::core::circus
