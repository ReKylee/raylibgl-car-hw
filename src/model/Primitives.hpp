#pragma once

#include <raylib.h>

// Small custom drawing helpers for things raylib has no single-call primitive
// for. The car's solids (box, cylinder) now use raylib built-ins directly
// (DrawCubeV / DrawCylinderEx).
namespace raylibgl::model {

    // XYZ axes drawn as three colored lines from the origin:
    // +X red, +Y green, +Z blue, each `length` units long. Unlit (lines).
    void drawAxes(float length);

} // namespace raylibgl::model
