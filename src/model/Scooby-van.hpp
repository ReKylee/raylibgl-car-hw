#pragma once

#include <raylib.h>

// The Scooby-Doo "Mystery Machine" van, built hierarchically from the part
// list in car-parts.md. Coordinates here match that table directly (Y up, the
// van faces -Z, ground at y = 0); drawCar() re-centers the whole model on the
// origin for the trackball.
//
// Each function honors the `wire` flag (the 'P' toggle) by forwarding it to the
// model::drawBox / drawCylinder helpers.
namespace raylibgl::model {

    // Filled body: the tapered turquoise solid (hexagonal side profile).
    void drawChassis(bool wire);

    // One tire: a dark cylinder centered at the local origin, axle along X.
    // drawCar places four of these.
    void drawWheel(bool wire);

    // Whole van, re-centered so its visual center (0, 1.24, 0) sits on the
    // origin (the trackball rotation pivot).
    void drawCar(bool wire);

} // namespace raylibgl::model
