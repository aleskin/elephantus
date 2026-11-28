#pragma once

#include "core/minigames/circus/objects/hoop.hpp"
#include "core/minigames/circus/objects/podium.hpp"
#include "core/text/kv_file.hpp"

#include <optional>
#include <variant>

namespace ce::core::circus
{

// Everything that can be placed on a level. A new kind of object = its own file in
// core/minigames/circus/objects/, one more alternative here and one line in parse_object().
using LevelObject = std::variant<Hoop, Podium>;

// Builds an object from its section ([hoop], [podium], ...); nullopt if the section is not an
// object.
std::optional<LevelObject> parse_object(const KvSection& section);

double object_x(const LevelObject& object);

} // namespace ce::core::circus
