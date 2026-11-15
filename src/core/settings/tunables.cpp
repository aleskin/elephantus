#include "core/settings/tunables.hpp"

#include <cmath>

namespace ce::core
{

double Tunables::jump_speed() const
{
    return std::sqrt(2.0 * gravity * jump_height);
}

double Tunables::air_time() const
{
    return 2.0 * jump_speed() / gravity;
}

} // namespace ce::core
