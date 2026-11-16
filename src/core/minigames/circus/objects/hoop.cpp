#include "core/minigames/circus/objects/hoop.hpp"

namespace ce::core::circus
{

double Hoop::bottom() const
{
    return height - radius;
}

double Hoop::top() const
{
    return height + radius;
}

Hoop parse_hoop(const KvSection& section)
{
    Hoop hoop;
    hoop.x = section.require_number("x");
    hoop.height = section.number("height", Hoop::default_height);
    hoop.radius = section.number("radius", Hoop::default_radius);
    if (hoop.radius <= 0.0)
    {
        throw KvError(section.find("radius")->line, "hoop radius must be above zero");
    }
    if (hoop.bottom() < 0.0)
    {
        throw KvError(section.line(), "hoop goes below the floor: height must be at least radius");
    }
    return hoop;
}

} // namespace ce::core::circus
