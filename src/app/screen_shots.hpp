#pragma once

#include "core/party/minigame.hpp"

#include <filesystem>
#include <vector>

namespace ce::app
{

// Saves every menu screen as <dir>/<screen>.png: title, main_menu, minigame_list. Lets the
// agent look at the menus without a person at the keyboard (`make shots`). The window must
// be open.
void save_screens(std::vector<core::MinigameInfo> minigames, const std::filesystem::path& dir);

} // namespace ce::app
