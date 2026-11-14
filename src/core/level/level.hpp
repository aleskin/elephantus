#pragma once

#include "core/objects/level_object.hpp"
#include "core/text/kv_file.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace ce::core
{

// Places a level can happen in. The look of each one lives in render/locations/.
bool is_known_location(std::string_view name);

// One level: where it happens and what stands on the path. The path runs from x = 0
// to x = length; the level is finished at x = length.
struct LevelData
{
    std::string title;
    std::string location;
    double length = 0.0;
    double start_x = 0.0;
    std::vector<LevelObject> objects; // sorted by x
};

// Level file: one [level] section (title, location, length, start) and one section
// per object ([hoop], ...).
LevelData parse_level(const KvDocument& document);
LevelData load_level(const std::filesystem::path& path);

// The order of levels: campaign.txt with one [level] section per level, file = level_01.txt.
struct Campaign
{
    std::vector<std::filesystem::path> levels;
};

Campaign parse_campaign(const KvDocument& document, const std::filesystem::path& folder);
Campaign load_campaign(const std::filesystem::path& path);

} // namespace ce::core
