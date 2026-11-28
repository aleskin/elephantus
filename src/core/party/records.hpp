#pragma once

#include "core/text/kv_file.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <string_view>

namespace ce::core
{

// The best points of every mini-game, kept between runs in records.txt in the user data
// folder: one section per mini-game id with "best = <points>".
class Records
{
public:
    // 0 for a mini-game that has no record yet.
    int best(std::string_view id) const;
    // Keeps the points if they beat the record; true when they did.
    bool offer(std::string_view id, int points);

    const std::map<std::string, int, std::less<>>& all() const;

private:
    std::map<std::string, int, std::less<>> best_;
};

Records parse_records(const KvDocument& document);
std::string format_records(const Records& records);
// No file yet = no records. Throws KvError when the file is broken.
Records load_records(const std::filesystem::path& path);

} // namespace ce::core
