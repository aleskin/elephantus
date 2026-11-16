#include "core/settings/config.hpp"

#include <algorithm>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

namespace ce::core
{

namespace
{

bool belongs_to_a_minigame(const std::string& section, std::span<const MinigameInfo> minigames)
{
    const std::size_t dot = section.find('.');
    if (dot == std::string::npos)
    {
        return false;
    }
    const std::string_view id = std::string_view{section}.substr(0, dot);
    return std::any_of(minigames.begin(), minigames.end(),
                       [id](const MinigameInfo& info)
                       {
                           return info.id == id;
                       });
}

} // namespace

GameConfig parse_config(KvDocument document, std::span<const MinigameInfo> minigames)
{
    GameConfig config;
    for (const KvSection& section : document.sections)
    {
        if (section.name().empty())
        {
            if (!section.entries().empty())
            {
                throw KvError(section.entries().front().line,
                              "put keys under a section, e.g. [pad]");
            }
            continue;
        }
        if (section.name() == "pad")
        {
            for (const KvEntry& entry : section.entries())
            {
                if (entry.key != "layout")
                {
                    throw KvError(entry.line, "unknown key '" + entry.key + "' in [pad]");
                }
                const std::optional<PadLayout> layout = pad_layout_from_name(entry.value);
                if (!layout)
                {
                    throw KvError(entry.line, "layout must be nintendo or xbox");
                }
                config.pad_layout = *layout;
            }
            continue;
        }
        if (!belongs_to_a_minigame(section.name(), minigames))
        {
            throw KvError(section.line(), "unknown section [" + section.name() + "]");
        }
    }
    for (const MinigameInfo& info : minigames)
    {
        if (info.check_config != nullptr)
        {
            info.check_config(document);
        }
    }
    config.document = std::move(document);
    return config;
}

GameConfig load_config(const std::filesystem::path& path, std::span<const MinigameInfo> minigames)
{
    try
    {
        return parse_config(load_kv_file(path), minigames);
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

ConfigWatcher::ConfigWatcher(std::filesystem::path path, std::vector<MinigameInfo> minigames,
                             double interval)
    : path_(std::move(path))
    , minigames_(std::move(minigames))
    , interval_(interval)
{
    check();
}

bool ConfigWatcher::poll(double dt)
{
    since_check_ += dt;
    if (since_check_ < interval_)
    {
        return false;
    }
    return check();
}

bool ConfigWatcher::check()
{
    since_check_ = 0.0;
    std::error_code code;
    const auto written = std::filesystem::last_write_time(path_, code);
    if (code)
    {
        seen_.reset();
        error_ = path_.filename().string() + ": file not found";
        return false;
    }
    if (seen_ && *seen_ == written)
    {
        return false;
    }
    seen_ = written;
    try
    {
        config_ = load_config(path_, minigames_);
        error_.clear();
        return true;
    }
    catch (const std::exception& error)
    {
        error_ = error.what();
        return false;
    }
}

const std::filesystem::path& ConfigWatcher::path() const
{
    return path_;
}

const GameConfig& ConfigWatcher::config() const
{
    return config_;
}

const std::string& ConfigWatcher::error() const
{
    return error_;
}

} // namespace ce::core
