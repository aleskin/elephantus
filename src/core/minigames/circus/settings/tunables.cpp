#include "core/minigames/circus/settings/tunables.hpp"

#include <cmath>

namespace ce::core::circus
{

double Tunables::jump_speed() const
{
    return std::sqrt(2.0 * gravity * jump_height);
}

double Tunables::air_time() const
{
    return 2.0 * jump_speed() / gravity;
}

} // namespace ce::core::circus
