#pragma once

#include "core/settings/tunables.hpp"
#include "core/text/kv_file.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace ce::core
{

// The game config (assets/config.txt): the numbers from Tunables, grouped in sections.
// A missing key keeps the value from the code; an unknown section or key is an error, so a
// typo does not go unnoticed. Every number must be above zero.
Tunables parse_tunables(const KvDocument& document);
Tunables load_tunables(const std::filesystem::path& path);

// Watches the config file while the game runs. The file is checked once per `interval`
// seconds; when it has changed, it is read again. A broken file keeps the last good
// numbers and leaves the reason in error().
class ConfigWatcher
{
public:
    explicit ConfigWatcher(std::filesystem::path path, double interval = 1.0);

    // True when new numbers were loaded during this call.
    bool poll(double dt);
    // Checks the file right away.
    bool check();

    const std::filesystem::path& path() const;
    const Tunables& tunables() const;
    // Empty when the last read went well.
    const std::string& error() const;

private:
    std::filesystem::path path_;
    double interval_;
    double since_check_ = 0.0;
    std::optional<std::filesystem::file_time_type> seen_;
    Tunables tunables_;
    std::string error_;
};

} // namespace ce::core
