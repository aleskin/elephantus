#pragma once

#include "core/input/input_frame.hpp"

#include <vector>

namespace ce::core
{

// Recognises buttons pressed one after another, each within `window` seconds of the
// previous one: X then B is the salto jump. A press of any other button starts over.
class Combo
{
public:
    Combo(std::vector<GameButton> sequence, double window);

    // True in the step where the last button of the sequence is pressed in time.
    bool step(const InputFrame& input, double dt);
    // Part of the sequence is pressed and there is still time for the rest.
    bool pending() const;
    void reset();

private:
    std::vector<GameButton> sequence_;
    double window_;
    std::size_t next_ = 0;
    double since_last_ = 0.0;
};

} // namespace ce::core
