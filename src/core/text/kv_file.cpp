#include "core/text/kv_file.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <fstream>
#include <sstream>

namespace ce::core
{

namespace
{

std::string_view trim(std::string_view text)
{
    const auto is_space = [](char c)
    {
        return std::isspace(static_cast<unsigned char>(c)) != 0;
    };
    while (!text.empty() && is_space(text.front()))
    {
        text.remove_prefix(1);
    }
    while (!text.empty() && is_space(text.back()))
    {
        text.remove_suffix(1);
    }
    return text;
}

std::string lower(std::string_view text)
{
    std::string result{text};
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c)
                   {
                       return static_cast<char>(std::tolower(c));
                   });
    return result;
}

bool valid_name(std::string_view name)
{
    if (name.empty())
    {
        return false;
    }
    return std::all_of(name.begin(), name.end(),
                       [](unsigned char c)
                       {
                           return std::isalnum(c) != 0 || c == '_';
                       });
}

// A section name is one or more names joined by dots: [hoop], [circus.elephant].
bool valid_section_name(std::string_view name)
{
    while (true)
    {
        const std::size_t dot = name.find('.');
        if (!valid_name(name.substr(0, dot)))
        {
            return false;
        }
        if (dot == std::string_view::npos)
        {
            return true;
        }
        name.remove_prefix(dot + 1);
    }
}

std::string where(const KvSection& section)
{
    if (section.name().empty())
    {
        return "";
    }
    return " in [" + section.name() + "]";
}

std::string format_error(int line, const std::string& detail, const std::string& file)
{
    std::string prefix = file;
    if (line > 0)
    {
        prefix += (prefix.empty() ? "line " : ", line ") + std::to_string(line);
    }
    return prefix.empty() ? detail : prefix + ": " + detail;
}

} // namespace

KvError::KvError(int line, const std::string& detail, const std::string& file)
    : std::runtime_error(format_error(line, detail, file))
    , line_(line)
    , detail_(detail)
    , file_(file)
{
}

int KvError::line() const
{
    return line_;
}

const std::string& KvError::detail() const
{
    return detail_;
}

const std::string& KvError::file() const
{
    return file_;
}

KvSection::KvSection(std::string name, int line)
    : name_(std::move(name))
    , line_(line)
{
}

const std::string& KvSection::name() const
{
    return name_;
}

int KvSection::line() const
{
    return line_;
}

const std::vector<KvEntry>& KvSection::entries() const
{
    return entries_;
}

void KvSection::add(KvEntry entry)
{
    if (const KvEntry* existing = find(entry.key))
    {
        throw KvError(entry.line, "key '" + entry.key + "' is already set on line " +
                                      std::to_string(existing->line) + where(*this));
    }
    entries_.push_back(std::move(entry));
}

const KvEntry* KvSection::find(std::string_view key) const
{
    const auto it = std::find_if(entries_.begin(), entries_.end(),
                                 [key](const KvEntry& entry)
                                 {
                                     return entry.key == key;
                                 });
    if (it == entries_.end())
    {
        return nullptr;
    }
    return &*it;
}

double KvSection::number(std::string_view key, double fallback) const
{
    const KvEntry* entry = find(key);
    if (entry == nullptr)
    {
        return fallback;
    }
    return parse_number(*entry);
}

double KvSection::require_number(std::string_view key) const
{
    const KvEntry* entry = find(key);
    if (entry == nullptr)
    {
        throw KvError(line_, "missing '" + std::string{key} + "'" + where(*this));
    }
    return parse_number(*entry);
}

std::string KvSection::text(std::string_view key, std::string_view fallback) const
{
    const KvEntry* entry = find(key);
    if (entry == nullptr)
    {
        return std::string{fallback};
    }
    return entry->value;
}

std::string KvSection::require_text(std::string_view key) const
{
    const KvEntry* entry = find(key);
    if (entry == nullptr || entry->value.empty())
    {
        throw KvError(line_, "missing '" + std::string{key} + "'" + where(*this));
    }
    return entry->value;
}

bool KvSection::flag(std::string_view key, bool fallback) const
{
    const KvEntry* entry = find(key);
    if (entry == nullptr)
    {
        return fallback;
    }
    return parse_flag(*entry);
}

std::vector<const KvSection*> KvDocument::all(std::string_view name) const
{
    std::vector<const KvSection*> result;
    for (const KvSection& section : sections)
    {
        if (section.name() == name)
        {
            result.push_back(&section);
        }
    }
    return result;
}

const KvSection* KvDocument::first(std::string_view name) const
{
    for (const KvSection& section : sections)
    {
        if (section.name() == name)
        {
            return &section;
        }
    }
    return nullptr;
}

KvDocument parse_kv(std::string_view text)
{
    KvDocument document;
    document.sections.emplace_back("", 0);

    int line_number = 0;
    while (!text.empty())
    {
        ++line_number;
        const std::size_t end = text.find('\n');
        std::string_view line = text.substr(0, end);
        text.remove_prefix(end == std::string_view::npos ? text.size() : end + 1);

        line = trim(line);
        if (line.empty() || line.front() == '#')
        {
            continue;
        }
        if (line.front() == '[')
        {
            if (line.back() != ']')
            {
                throw KvError(line_number, "section must look like [name]");
            }
            const std::string_view name = trim(line.substr(1, line.size() - 2));
            if (!valid_section_name(name))
            {
                throw KvError(line_number, "bad section name '" + std::string{name} + "'");
            }
            document.sections.emplace_back(std::string{name}, line_number);
            continue;
        }
        const std::size_t equals = line.find('=');
        if (equals == std::string_view::npos)
        {
            throw KvError(line_number, "expected 'key = value'");
        }
        const std::string_view key = trim(line.substr(0, equals));
        if (!valid_name(key))
        {
            throw KvError(line_number, "bad key '" + std::string{key} + "'");
        }
        document.sections.back().add(
            KvEntry{std::string{key}, std::string{trim(line.substr(equals + 1))}, line_number});
    }
    return document;
}

KvDocument load_kv_file(const std::filesystem::path& path)
{
    const std::string name = path.filename().string();
    std::ifstream file{path, std::ios::binary};
    if (!file)
    {
        throw KvError(0, "cannot open " + path.string(), name);
    }
    std::ostringstream content;
    content << file.rdbuf();
    try
    {
        return parse_kv(content.str());
    }
    catch (const KvError& error)
    {
        throw KvError(error.line(), error.detail(), name);
    }
}

double parse_number(const KvEntry& entry)
{
    std::string value = entry.value;
    std::replace(value.begin(), value.end(), ',', '.');
    double result = 0.0;
    const char* begin = value.data();
    const char* end = value.data() + value.size();
    if (!value.empty() && *begin == '+')
    {
        ++begin;
    }
    const auto [ptr, ec] = std::from_chars(begin, end, result);
    if (value.empty() || ec != std::errc{} || ptr != end || !std::isfinite(result))
    {
        throw KvError(entry.line,
                      "'" + entry.key + "' must be a number, got '" + entry.value + "'");
    }
    return result;
}

bool parse_flag(const KvEntry& entry)
{
    const std::string value = lower(entry.value);
    if (value == "yes" || value == "true" || value == "on" || value == "1")
    {
        return true;
    }
    if (value == "no" || value == "false" || value == "off" || value == "0")
    {
        return false;
    }
    throw KvError(entry.line, "'" + entry.key + "' must be yes or no, got '" + entry.value + "'");
}

} // namespace ce::core
