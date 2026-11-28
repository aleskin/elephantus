#include "core/party/records.hpp"

#include <cmath>
#include <sstream>

namespace ce::core
{

int Records::best(std::string_view id) const
{
    const auto found = best_.find(id);
    return found == best_.end() ? 0 : found->second;
}

bool Records::offer(std::string_view id, int points)
{
    if (points <= best(id))
    {
        return false;
    }
    best_.insert_or_assign(std::string{id}, points);
    return true;
}

const std::map<std::string, int, std::less<>>& Records::all() const
{
    return best_;
}

Records parse_records(const KvDocument& document)
{
    Records records;
    for (const KvSection& section : document.sections)
    {
        if (section.name().empty())
        {
            if (!section.entries().empty())
            {
                throw KvError(section.entries().front().line,
                              "put keys under a section, e.g. [circus]");
            }
            continue;
        }
        const double best = section.require_number("best");
        if (best < 0.0)
        {
            throw KvError(section.find("best")->line, "best must not be below zero");
        }
        records.offer(section.name(), static_cast<int>(std::lround(best)));
    }
    return records;
}

std::string format_records(const Records& records)
{
    std::ostringstream out;
    out << "# Elephantus: the best points of every mini-game.\n";
    for (const auto& [id, best] : records.all())
    {
        out << "\n[" << id << "]\nbest = " << best << "\n";
    }
    return out.str();
}

Records load_records(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
    {
        return {};
    }
    return parse_records(load_kv_file(path));
}

} // namespace ce::core
