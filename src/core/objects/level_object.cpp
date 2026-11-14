#include "core/objects/level_object.hpp"

namespace ce::core
{

std::optional<LevelObject> parse_object(const KvSection& section)
{
    if (section.name() == "hoop")
    {
        return parse_hoop(section);
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

} // namespace ce::core
