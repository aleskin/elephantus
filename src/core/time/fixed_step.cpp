#include "core/time/fixed_step.hpp"

#include <algorithm>

namespace ce::core
{

FixedStep::FixedStep(double rate)
    : dt_(1.0 / rate)
{
}

double FixedStep::dt() const
{
    return dt_;
}

int FixedStep::advance(double frame_time)
{
    accumulated_ += std::clamp(frame_time, 0.0, max_frame_time);
    int steps = 0;
    while (accumulated_ >= dt_)
    {
        accumulated_ -= dt_;
        ++steps;
    }
    return steps;
}

} // namespace ce::core
