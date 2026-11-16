#pragma once

#include "render/minigame_play.hpp"

#include <filesystem>

namespace ce::app
{

// Plays a script against a mini-game (its first player) with a fixed step and saves the
// requested frames as PNG files <shots_dir>/<script name>_<shot name>.png. The window
// must be open.
void run_script(render::MinigamePlay& play, const std::filesystem::path& script_path,
                const std::filesystem::path& shots_dir);

} // namespace ce::app
