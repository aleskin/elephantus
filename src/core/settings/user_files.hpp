#pragma once

#include <filesystem>
#include <string_view>

namespace ce::core
{

// Files of the player, outside the repository. Settings: $XDG_CONFIG_HOME/elephantus or
// ~/.config/elephantus; progress and records: $XDG_DATA_HOME/elephantus or
// ~/.local/share/elephantus. On Windows both are %APPDATA%\elephantus. Empty when the
// home folder is unknown: then nothing is kept between runs.
std::filesystem::path user_config_dir();
std::filesystem::path user_data_dir();

// Writes the whole file at once (a temporary file, then a rename), creating folders.
// Throws std::runtime_error when it cannot.
void write_text_file(const std::filesystem::path& path, std::string_view text);

} // namespace ce::core
