#pragma once

#include <raylib.h>

// Small custom drawing helpers for the car model. Boxes use raylib's cube
// primitive directly; cylinders are emitted locally along +Y with normals so
// they shade correctly under the lighting shader. Callers still use rlgl
// matrices to place and rotate parts in the model hierarchy.
namespace raylibgl::model {

    // XYZ axes drawn as three colored lines from the origin:
    // +X red, +Y green, +Z blue, each `length` units long. Unlit (lines).
    void drawAxes(float length);

    // Box centered at `center` with full `size` (x=width, y=height, z=depth).
    // Draws filled, or as wireframe edges when `wire` is true. Wraps raylib's
    // DrawCubeV / DrawCubeWiresV.
    void drawBox(Vector3 center, Vector3 size, Color color, bool wire);

    // Canonical low-poly cylinder centered at `center`, with its axis along local +Y.
    // Callers rotate the current matrix before this call when the cylinder
    // should point along X or Z. The filled path emits faceted normals for a
    // blockier low-poly look.
    void drawCylinder(Vector3 center, float radius, float height, Color color, bool wire, int sides = 12);

} // namespace raylibgl::model
