#include "render/screens/menu.hpp"
#include "render/screens/screens.hpp"

#include <string>
#include <vector>

namespace ce::render
{

namespace
{

std::string label(core::MainMenuItem item)
{
    switch (item)
    {
    case core::MainMenuItem::Minigames:
        return "Mini-games";
    case core::MainMenuItem::Quit:
        return "Quit";
    }
    return "?";
}

} // namespace

void draw_main_menu(const core::PartyFlow& flow)
{
    std::vector<std::string> items;
    for (const core::MainMenuItem item : core::main_menu_items)
    {
        items.push_back(label(item));
    }
    draw_menu("Elephantus", items, flow.cursor(), "Up / Down - move     A - choose     B - back");
}

} // namespace ce::render
