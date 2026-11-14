#pragma once

namespace ce::core
{

// Logic runs in equal steps whatever the frame rate, so the same presses always give
// the same game. Collects real frame time and says how many logic steps to run.
class FixedStep
{
public:
    static constexpr double default_rate = 120.0;
    // A long stall (window dragged, debugger) does not turn into a burst of steps.
    static constexpr double max_frame_time = 0.25;

    explicit FixedStep(double rate = default_rate);

    double dt() const;
    int advance(double frame_time);

private:
    double dt_;
    double accumulated_ = 0.0;
};

} // namespace ce::core
