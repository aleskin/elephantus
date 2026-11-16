#pragma once

namespace ce::core::circus
{

enum class ElephantState
{
    Standing, // at the start, on four legs, waiting for A
    Walking,  // on two legs while A is held
    Jumping,  // in the air, turning the salto
    Fallen,   // back on four legs: the level starts over
};

// The elephant on the path.
struct Elephant
{
    // TODO(CEL-00018): body size of the grey block, until the elephant gets a shape.
    static constexpr double length = 1.6;
    static constexpr double height = 1.2;
    // TODO(CEL-00018): the elephant touches things as a ball of this radius (curled up
    // in the salto, its feet on the floor when walking).
    static constexpr double body_radius = 0.45;

    ElephantState state = ElephantState::Standing;
    double x = 0.0;  // middle of the body along the path
    double y = 0.0;  // bottom of the body above the floor
    double vy = 0.0; // speed up, metres per second
    double air_time = 0.0;
    double salto = 0.0;     // how much of the turn is done: 0 at the take-off, 1 at landing
    double without_a = 0.0; // seconds on the floor without A and without a jump

    double centre_y() const
    {
        return y + body_radius;
    }
};

} // namespace ce::core::circus
