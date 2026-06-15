#pragma once

#include <raylib.h>

// Low-level modeling primitives, authored vertex-by-vertex with rlgl immediate
// mode (rlBegin / rlVertex3f / rlNormal3f). These are the building blocks the
// car is assembled from - the rlgl equivalent of the assignment's hand-built
// polygons and GLU quadrics. Every function draws in its OWN local space,
// centered on the origin, so callers position them with the matrix stack
// (rlPushMatrix / rlTranslatef / rlRotatef / rlScalef).
namespace raylibgl::model {

    // Axis-aligned box centered at the origin, with the given full size
    // (width x, height y, depth z). Per-face normals are set so lighting will
    // work once it's added. Faces wind counter-clockwise when viewed from
    // outside, matching raylib's default front-face / back-face culling.
    void drawBox(Vector3 size, Color color);

    // XYZ axes drawn as three colored lines from the origin:
    // +X red, +Y green, +Z blue, each `length` units long. Unlit (lines).
    void drawAxes(float length);

    // Flat disk in the XZ plane at the origin, facing +Y. `sides` controls how
    // round it looks (more = smoother). The assignment's GLU "disk" quadric.
    void drawDisk(float radius, int sides, Color color);

    // Solid cylinder centered at the origin, axis along Y, spanning
    // [-height/2, +height/2], capped at both ends by disks ("cylinder bounded
    // by disks"). Side normals point radially out, caps point along +/-Y, so
    // it's ready for lighting. Rotate it with the matrix stack to lay the axis
    // along X (wheels) or Z (lights).
    void drawCylinder(float radius, float height, int sides, Color color);

} // namespace raylibgl::model
