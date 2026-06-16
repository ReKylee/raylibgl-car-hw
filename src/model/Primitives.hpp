#pragma once

#include <raylib.h>

// Small custom drawing helpers for things raylib has no single-call primitive
// for. The car's solids (box, cylinder) now use raylib built-ins directly
// (DrawCubeV / DrawCylinderEx).
namespace raylibgl::model {

    // XYZ axes drawn as three colored lines from the origin:
    // +X red, +Y green, +Z blue, each `length` units long. Unlit (lines).
    void drawAxes(float length);

    // Box centered at `center` with full `size` (x=width, y=height, z=depth).
    // Draws filled, or as wireframe edges when `wire` is true. Wraps raylib's
    // DrawCubeV / DrawCubeWiresV.
    void drawBox(Vector3 center, Vector3 size, Color color, bool wire);

    // Cylinder between its two end-cap centers `start` and `end`, with the given
    // `radius`. Filled, or wireframe when `wire` is true. `sides` sets how round
    // it looks. Wraps raylib's DrawCylinderEx / DrawCylinderWiresEx.
    void drawCylinder(Vector3 start, Vector3 end, float radius, Color color, bool wire, int sides = 16);

} // namespace raylibgl::model
