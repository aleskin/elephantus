#pragma once

#include "core/game/game.hpp"

#include <filesystem>

namespace ce::app
{

// Plays a script against the game with a fixed step and saves the requested frames as
// PNG files <shots_dir>/<script name>_<shot name>.png. The window must be open.
void run_script(core::Game& game, const std::filesystem::path& script_path,
                const std::filesystem::path& shots_dir);

} // namespace ce::app
