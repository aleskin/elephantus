#include "core/minigames/circus/objects/level_object.hpp"

namespace ce::core::circus
{

std::optional<LevelObject> parse_object(const KvSection& section)
{
    if (section.name() == "hoop")
    {
        return parse_hoop(section);
    }
    if (section.name() == "podium")
    {
        return parse_podium(section);
    }
    return std::nullopt;
}

double object_x(const LevelObject& object)
{
    return std::visit(
        [](const auto& item)
        {
            return item.x;
        },
        object);
}

} // namespace ce::core::circus
