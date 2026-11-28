#pragma once

namespace ce::core::circus
{

// All game numbers that are not decided yet, in one place. The game reads them from the
// game config assets/config.txt (settings/config); these defaults are used for
// missing keys and by the screenshot scripts, so the shots do not depend on the file.
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
    // Owner (2026-11-28): points for every salto, forward or backward, with or without a
    // hoop. TODO(CEL-00064): 5 from the earlier "5 for a jump"; TODO(CEL-00055): fewer for
    // a jump that is not perfect.
    double salto_points = 5.0;
    // Owner (2026-11-28): a salto through a hoop gives this many times the points.
    double hoop_multiplier = 2.0;
    // Owner (2026-11-27): attempts in one run; a fall or a touched hoop takes one.
    double attempts = 3.0;

    // Speed straight up at the start of a jump and the time in the air.
    double jump_speed() const;
    double air_time() const;
    // Points for a salto that flew through `hoops` hoops; whole points.
    int salto_score(int hoops) const;
    // At least one attempt.
    int attempt_count() const;
};

} // namespace ce::core::circus
