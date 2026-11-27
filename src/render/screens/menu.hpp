#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace ce::render
{

// A menu screen: a heading, items one under another with the chosen one highlighted, and
// a hint about the buttons at the bottom.
void draw_menu(const std::string& heading, const std::vector<std::string>& items,
               std::size_t cursor, const std::string& hint);

} // namespace ce::render
