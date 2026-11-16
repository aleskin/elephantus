#include "app/minigames.hpp"

#include "core/minigames/circus/circus.hpp"
#include "render/minigames/circus/circus_play.hpp"

#include <array>

namespace ce::app
{

namespace
{

const std::array<MinigameModule, 1> modules = {{
    {&core::circus::info, &render::circus::start},
}};

} // namespace

std::span<const MinigameModule> minigames()
{
    return modules;
}

const MinigameModule* find_minigame(std::string_view id)
{
    for (const MinigameModule& module : modules)
    {
        if (module.info->id == id)
        {
            return &module;
        }
    }
    return nullptr;
}

std::vector<core::MinigameInfo> minigame_infos()
{
    std::vector<core::MinigameInfo> infos;
    for (const MinigameModule& module : modules)
    {
        infos.push_back(*module.info);
    }
    return infos;
}

} // namespace ce::app
