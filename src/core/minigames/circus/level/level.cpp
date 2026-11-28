#include "core/minigames/circus/level/level.hpp"

#include <algorithm>
#include <array>
#include <variant>

namespace ce::core::circus
{

namespace
{

constexpr std::array<std::string_view, 1> known_locations = {"arena"};

void reject_loose_keys(const KvDocument& document, std::string_view section)
{
    const KvSection* root = document.first("");
    if (root != nullptr && !root->entries().empty())
    {
        throw KvError(root->entries().front().line,
                      "put keys under a section, e.g. [" + std::string{section} + "]");
    }
}

template <typename Parse> auto with_file_name(const std::filesystem::path& path, Parse parse)
{
    try
    {
        return parse();
    }
    catch (const KvError& error)
    {
        if (!error.file().empty())
        {
            throw;
        }
        throw KvError(error.line(), error.detail(), path.filename().string());
    }
}

} // namespace

bool is_known_location(std::string_view name)
{
    return std::find(known_locations.begin(), known_locations.end(), name) != known_locations.end();
}

LevelData parse_level(const KvDocument& document)
{
    reject_loose_keys(document, "level");

    const std::vector<const KvSection*> headers = document.all("level");
    if (headers.empty())
    {
        throw KvError(0, "missing [level] section");
    }
    if (headers.size() > 1)
    {
        throw KvError(headers[1]->line(), "only one [level] section is allowed");
    }
    const KvSection& header = *headers.front();

    LevelData level;
    level.title = header.text("title", "");
    level.location = header.require_text("location");
    if (!is_known_location(level.location))
    {
        throw KvError(header.find("location")->line, "unknown location '" + level.location + "'");
    }
    level.length = header.require_number("length");
    if (level.length <= 0.0)
    {
        throw KvError(header.find("length")->line, "length must be above zero");
    }
    level.start_x = header.number("start", 0.0);
    if (level.start_x < 0.0 || level.start_x >= level.length)
    {
        throw KvError(header.find("start")->line, "start must be inside the level");
    }

    bool has_podium = false;
    for (const KvSection& section : document.sections)
    {
        if (section.name().empty() || &section == &header)
        {
            continue;
        }
        std::optional<LevelObject> object = parse_object(section);
        if (!object)
        {
            throw KvError(section.line(), "unknown section [" + section.name() + "]");
        }
        const double x = object_x(*object);
        if (x < 0.0 || x > level.length)
        {
            throw KvError(section.find("x")->line, "x must be between 0 and the level length");
        }
        if (const Podium* podium = std::get_if<Podium>(&*object))
        {
            if (has_podium)
            {
                throw KvError(section.line(), "only one [podium] per level");
            }
            has_podium = true;
            if (podium->right() > level.length)
            {
                throw KvError(section.find("x")->line, "podium goes past the level end");
            }
        }
        level.objects.push_back(std::move(*object));
    }
    std::stable_sort(level.objects.begin(), level.objects.end(),
                     [](const LevelObject& a, const LevelObject& b)
                     {
                         return object_x(a) < object_x(b);
                     });
    return level;
}

LevelData load_level(const std::filesystem::path& path)
{
    return with_file_name(path,
                          [&path]
                          {
                              return parse_level(load_kv_file(path));
                          });
}

Campaign parse_campaign(const KvDocument& document, const std::filesystem::path& folder)
{
    reject_loose_keys(document, "level");

    Campaign campaign;
    for (const KvSection& section : document.sections)
    {
        if (section.name().empty())
        {
            continue;
        }
        if (section.name() != "level")
        {
            throw KvError(section.line(), "unknown section [" + section.name() + "]");
        }
        campaign.levels.push_back(folder / section.require_text("file"));
    }
    if (campaign.levels.empty())
    {
        throw KvError(0, "no levels: add a [level] section with file = ...");
    }
    return campaign;
}

Campaign load_campaign(const std::filesystem::path& path)
{
    return with_file_name(path,
                          [&path]
                          {
                              return parse_campaign(load_kv_file(path), path.parent_path());
                          });
}

} // namespace ce::core::circus
