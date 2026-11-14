#include "core/game_info.hpp"

namespace ce::core
{

std::string_view game_title()
{
    return "Circus Elephant";
}

std::string_view game_version()
{
    return CE_VERSION;
}

} // namespace ce::core
