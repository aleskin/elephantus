#pragma once

namespace ce::core
{

// The elephant on the path. For now it only stands still; walking and jumping come
// with the grey prototype (CEL-00013).
struct Elephant
{
    // TODO(CEL-00013): body size of the grey block, until the elephant gets a shape.
    static constexpr double length = 1.6;
    static constexpr double height = 1.2;

    double x = 0.0; // middle of the body along the path
    double y = 0.0; // feet above the floor
};

} // namespace ce::core
