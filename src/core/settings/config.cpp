#include "core/settings/config.hpp"

#include <array>
#include <string_view>
#include <system_error>
#include <utility>

namespace ce::core
{

namespace
{

struct Knob
{
    std::string_view section;
    std::string_view key;
    double Tunables::* value;
};

constexpr std::array<Knob, 6> knobs = {{
    {"elephant", "walk_speed", &Tunables::walk_speed},
    {"elephant", "jump_height", &Tunables::jump_height},
    {"elephant", "gravity", &Tunables::gravity},
    {"elephant", "fall_grace", &Tunables::fall_grace},
    {"buttons", "salto_window", &Tunables::salto_window},
    {"game", "restart_delay", &Tunables::restart_delay},
}};

const Knob* find_knob(std::string_view section, std::string_view key)
{
    for (const Knob& knob : knobs)
    {
        if (knob.section == section && knob.key == key)
        {
            return &knob;
        }
    }
    return nullptr;
}

bool known_section(std::string_view section)
{
    for (const Knob& knob : knobs)
    {
        if (knob.section == section)
        {
            return true;
        }
    }
    return false;
}

} // namespace

Tunables parse_tunables(const KvDocument& document)
{
    Tunables tunables;
    for (const KvSection& section : document.sections)
    {
        if (section.name().empty())
        {
            if (!section.entries().empty())
            {
                throw KvError(section.entries().front().line,
                              "put keys under a section, e.g. [elephant]");
            }
            continue;
        }
        if (!known_section(section.name()))
        {
            throw KvError(section.line(), "unknown section [" + section.name() + "]");
        }
        for (const KvEntry& entry : section.entries())
        {
            const Knob* knob = find_knob(section.name(), entry.key);
            if (knob == nullptr)
            {
                throw KvError(entry.line,
                              "unknown key '" + entry.key + "' in [" + section.name() + "]");
            }
            const double value = parse_number(entry);
            if (!(value > 0.0))
            {
                throw KvError(entry.line, entry.key + " must be above zero");
            }
            tunables.*(knob->value) = value;
        }
    }
    return tunables;
}

Tunables load_tunables(const std::filesystem::path& path)
{
    try
    {
        return parse_tunables(load_kv_file(path));
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

ConfigWatcher::ConfigWatcher(std::filesystem::path path, double interval)
    : path_(std::move(path))
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
        tunables_ = load_tunables(path_);
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

const Tunables& ConfigWatcher::tunables() const
{
    return tunables_;
}

const std::string& ConfigWatcher::error() const
{
    return error_;
}

} // namespace ce::core
