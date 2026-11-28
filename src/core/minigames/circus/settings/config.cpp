#include "core/minigames/circus/settings/config.hpp"

#include <array>
#include <string_view>

namespace ce::core::circus
{

namespace
{

constexpr std::string_view prefix = "circus.";

struct Knob
{
    std::string_view section;
    std::string_view key;
    double Tunables::* value;
};

constexpr std::array<Knob, 9> knobs = {{
    {"circus.elephant", "walk_speed", &Tunables::walk_speed},
    {"circus.elephant", "jump_height", &Tunables::jump_height},
    {"circus.elephant", "gravity", &Tunables::gravity},
    {"circus.elephant", "fall_grace", &Tunables::fall_grace},
    {"circus.buttons", "salto_window", &Tunables::salto_window},
    {"circus.game", "restart_delay", &Tunables::restart_delay},
    {"circus.game", "salto_points", &Tunables::salto_points},
    {"circus.game", "hoop_multiplier", &Tunables::hoop_multiplier},
    {"circus.game", "attempts", &Tunables::attempts},
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

Tunables read_tunables(const KvDocument& config)
{
    Tunables tunables;
    for (const KvSection& section : config.sections)
    {
        if (!section.name().starts_with(prefix))
        {
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

void check_config(const KvDocument& config)
{
    read_tunables(config);
}

} // namespace ce::core::circus
