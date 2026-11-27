#include "render/screens/menu.hpp"
#include "render/screens/screens.hpp"

#include <string>
#include <vector>

namespace ce::render
{

void draw_minigame_list(const core::PartyFlow& flow)
{
    std::vector<std::string> items;
    for (const core::MinigameInfo& info : flow.minigames())
    {
        items.emplace_back(info.title);
    }
    draw_menu("Мини-игры", items, flow.cursor(), "A — играть     B — назад     Esc в игре — сюда");
}

} // namespace ce::render
