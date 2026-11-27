#include "core/settings/user_files.hpp"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>

namespace ce::core
{

namespace
{

constexpr const char* folder = "elephantus";

std::filesystem::path from_env(const char* name)
{
    const char* value = std::getenv(name);
    if (value == nullptr || *value == '\0')
    {
        return {};
    }
    return std::filesystem::path{value};
}

std::filesystem::path user_dir(const char* xdg_name, const char* home_relative)
{
#ifdef _WIN32
    (void)xdg_name;
    (void)home_relative;
    const std::filesystem::path appdata = from_env("APPDATA");
    return appdata.empty() ? appdata : appdata / folder;
#else
    const std::filesystem::path xdg = from_env(xdg_name);
    if (!xdg.empty())
    {
        return xdg / folder;
    }
    const std::filesystem::path home = from_env("HOME");
    return home.empty() ? home : home / home_relative / folder;
#endif
}

} // namespace

std::filesystem::path user_config_dir()
{
    return user_dir("XDG_CONFIG_HOME", ".config");
}

std::filesystem::path user_data_dir()
{
    return user_dir("XDG_DATA_HOME", ".local/share");
}

void write_text_file(const std::filesystem::path& path, std::string_view text)
{
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path());
    }
    std::filesystem::path temporary = path;
    temporary += ".tmp";
    {
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        out.write(text.data(), static_cast<std::streamsize>(text.size()));
        if (!out)
        {
            throw std::runtime_error("cannot write " + temporary.string());
        }
    }
    std::filesystem::rename(temporary, path);
}

} // namespace ce::core
