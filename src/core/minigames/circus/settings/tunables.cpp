#include "core/minigames/circus/settings/tunables.hpp"

#include <algorithm>
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

int Tunables::points_per_hoop() const
{
    return static_cast<int>(std::lround(hoop_points));
}

int Tunables::attempt_count() const
{
    return std::max(1, static_cast<int>(std::lround(attempts)));
}

} // namespace ce::core::circus
