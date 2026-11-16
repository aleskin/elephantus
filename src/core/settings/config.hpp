#pragma once

#include "core/input/pad_layout.hpp"
#include "core/party/minigame.hpp"
#include "core/text/kv_file.hpp"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace ce::core
{

// Everything the game config (assets/config.txt) sets.
struct GameConfig
{
    PadLayout pad_layout = PadLayout::Nintendo;
    // The whole file: every mini-game reads its own [<id>.something] sections from it.
    KvDocument document;
};

// [pad] layout for the whole game, and sections of the mini-games, each checked by its
// own check_config. An unknown section or key is an error, so a typo does not go
// unnoticed.
GameConfig parse_config(KvDocument document, std::span<const MinigameInfo> minigames);
GameConfig load_config(const std::filesystem::path& path, std::span<const MinigameInfo> minigames);

// Watches the config file while the game runs. The file is checked once per `interval`
// seconds; when it has changed, it is read again. A broken file keeps the last good
// config and leaves the reason in error().
class ConfigWatcher
{
public:
    ConfigWatcher(std::filesystem::path path, std::vector<MinigameInfo> minigames,
                  double interval = 1.0);

    // True when a new config was loaded during this call.
    bool poll(double dt);
    // Checks the file right away.
    bool check();

    const std::filesystem::path& path() const;
    const GameConfig& config() const;
    // Empty when the last read went well.
    const std::string& error() const;

private:
    std::filesystem::path path_;
    std::vector<MinigameInfo> minigames_;
    double interval_;
    double since_check_ = 0.0;
    std::optional<std::filesystem::file_time_type> seen_;
    GameConfig config_;
    std::string error_;
};

} // namespace ce::core
