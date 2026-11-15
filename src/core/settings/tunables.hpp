#pragma once

namespace ce::core
{

// All game numbers that are not decided yet, in one place. Later they come from the
// "Polina's console" file (CEL-00021); until then these defaults are used.
struct Tunables
{
    // TODO(CEL-00031): walking speed on two legs, metres per second; tune with Polina.
    double walk_speed = 3.0;
    // TODO(CEL-00031): how high the salto jump lifts the elephant, metres.
    double jump_height = 1.3;
    // TODO(CEL-00031): how fast the elephant comes down, metres per second squared.
    double gravity = 20.0;
    // TODO(CEL-00010): "very fast" - the longest pause between X and B, seconds.
    double salto_window = 0.4;
    // TODO(CEL-00029): time without A and without a jump before the elephant falls, seconds.
    double fall_grace = 0.3;
    // TODO(CEL-00031): pause after a fall or a touched hoop before the level starts over.
    double restart_delay = 1.0;

    // Speed straight up at the start of a jump and the time in the air.
    double jump_speed() const;
    double air_time() const;
};

} // namespace ce::core
