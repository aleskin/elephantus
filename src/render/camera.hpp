#pragma once

#include <raylib.h>

namespace ce::render
{

// Perspective camera from the side of the path, looking at focus_x.
Camera3D side_camera(float focus_x);

} // namespace ce::render
