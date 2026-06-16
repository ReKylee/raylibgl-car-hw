#include "model/Primitives.hpp"

#include <rlgl.h>

namespace raylibgl::model {

    void drawAxes(float length) {
        rlBegin(RL_LINES);

        // +X red
        rlColor4ub(230, 41, 55, 255);
        rlVertex3f(0.0f, 0.0f, 0.0f);
        rlVertex3f(length, 0.0f, 0.0f);

        // +Y green
        rlColor4ub(0, 228, 48, 255);
        rlVertex3f(0.0f, 0.0f, 0.0f);
        rlVertex3f(0.0f, length, 0.0f);

        // +Z blue
        rlColor4ub(0, 121, 241, 255);
        rlVertex3f(0.0f, 0.0f, 0.0f);
        rlVertex3f(0.0f, 0.0f, length);

        rlEnd();
    }

    void drawBox(Vector3 center, Vector3 size, Color color, bool wire) {
        if (wire) {
            DrawCubeWiresV(center, size, color);
        } else {
            DrawCubeV(center, size, color);
        }
    }

    void drawCylinder(Vector3 start, Vector3 end, float radius, Color color, bool wire, int sides) {
        if (wire) {
            DrawCylinderWiresEx(start, end, radius, radius, sides, color);
        } else {
            DrawCylinderEx(start, end, radius, radius, sides, color);
        }
    }

} // namespace raylibgl::model
