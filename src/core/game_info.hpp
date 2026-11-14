#pragma once

#include <string_view>

namespace ce::core
{

std::string_view game_title();
// MAJOR.MINOR.PATCH from Makefile.version.
std::string_view game_version();

} // namespace ce::core
