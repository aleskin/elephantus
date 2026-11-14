#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ce::core
{

// Error in a settings or level file: "level_01.txt, line 3: <detail>".
// line() is 1-based, 0 when the file itself is the problem.
class KvError : public std::runtime_error
{
public:
    KvError(int line, const std::string& detail, const std::string& file = "");

    int line() const;
    const std::string& detail() const;
    const std::string& file() const;

private:
    int line_;
    std::string detail_;
    std::string file_;
};

struct KvEntry
{
    std::string key;
    std::string value;
    int line = 0;
};

// One [section]; the same name may repeat (one [hoop] per hoop).
class KvSection
{
public:
    KvSection(std::string name, int line);

    const std::string& name() const;
    int line() const;
    const std::vector<KvEntry>& entries() const;

    void add(KvEntry entry);
    const KvEntry* find(std::string_view key) const;

    double number(std::string_view key, double fallback) const;
    double require_number(std::string_view key) const;
    std::string text(std::string_view key, std::string_view fallback) const;
    std::string require_text(std::string_view key) const;
    bool flag(std::string_view key, bool fallback) const;

private:
    std::string name_;
    int line_;
    std::vector<KvEntry> entries_;
};

// The whole file. Keys before the first [section] go to a section with an empty name.
struct KvDocument
{
    std::vector<KvSection> sections;

    std::vector<const KvSection*> all(std::string_view name) const;
    const KvSection* first(std::string_view name) const;
};

// Format: "# comment" lines, "[section]" lines, "key = value" lines, blank lines.
KvDocument parse_kv(std::string_view text);
KvDocument load_kv_file(const std::filesystem::path& path);

// "1.5" and "1,5" are both accepted: the file is edited by hand.
double parse_number(const KvEntry& entry);
// yes/no, true/false, on/off, 1/0.
bool parse_flag(const KvEntry& entry);

} // namespace ce::core
