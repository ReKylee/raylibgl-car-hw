#include "model/Primitives.hpp"

#include <rlgl.h>

#include <cmath>

namespace raylibgl::model {

    void drawBox(Vector3 size, Color color) {
        // Half-extents: the box spans [-h, +h] on each axis around the origin.
        const float hx = size.x * 0.5f;
        const float hy = size.y * 0.5f;
        const float hz = size.z * 0.5f;

        rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // Each face: one normal, then 4 corners wound counter-clockwise as seen
        // from OUTSIDE the box (so it survives back-face culling).

        // Front face (+Z)
        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlVertex3f(-hx, -hy, hz);
        rlVertex3f(hx, -hy, hz);
        rlVertex3f(hx, hy, hz);
        rlVertex3f(-hx, hy, hz);

        // Back face (-Z)
        rlNormal3f(0.0f, 0.0f, -1.0f);
        rlVertex3f(hx, -hy, -hz);
        rlVertex3f(-hx, -hy, -hz);
        rlVertex3f(-hx, hy, -hz);
        rlVertex3f(hx, hy, -hz);

        // Right face (+X)
        rlNormal3f(1.0f, 0.0f, 0.0f);
        rlVertex3f(hx, -hy, hz);
        rlVertex3f(hx, -hy, -hz);
        rlVertex3f(hx, hy, -hz);
        rlVertex3f(hx, hy, hz);

        // Left face (-X)
        rlNormal3f(-1.0f, 0.0f, 0.0f);
        rlVertex3f(-hx, -hy, -hz);
        rlVertex3f(-hx, -hy, hz);
        rlVertex3f(-hx, hy, hz);
        rlVertex3f(-hx, hy, -hz);

        // Top face (+Y)
        rlNormal3f(0.0f, 1.0f, 0.0f);
        rlVertex3f(-hx, hy, hz);
        rlVertex3f(hx, hy, hz);
        rlVertex3f(hx, hy, -hz);
        rlVertex3f(-hx, hy, -hz);

        // Bottom face (-Y)
        rlNormal3f(0.0f, -1.0f, 0.0f);
        rlVertex3f(-hx, -hy, -hz);
        rlVertex3f(hx, -hy, -hz);
        rlVertex3f(hx, -hy, hz);
        rlVertex3f(-hx, -hy, hz);

        rlEnd();
    }

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

    void drawDisk(float radius, int sides, Color color) {
        rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlNormal3f(0.0f, 1.0f, 0.0f); // faces +Y

        for (int i = 0; i < sides; i++) {
            const float a0 = 2.0f * PI * static_cast<float>(i) / sides;
            const float a1 = 2.0f * PI * static_cast<float>(i + 1) / sides;

            // Wound CCW as seen from +Y (so the +Y side is the front face). The
            // -Z on the rim points "up" on screen when looking down the Y axis.
            rlVertex3f(0.0f, 0.0f, 0.0f);
            rlVertex3f(radius * std::cos(a0), 0.0f, -radius * std::sin(a0));
            rlVertex3f(radius * std::cos(a1), 0.0f, -radius * std::sin(a1));
        }

        rlEnd();
    }

    void drawCylinder(float radius, float height, int sides, Color color) {
        const float h2 = height * 0.5f;

        // --- side wall: one quad per segment, normals pointing radially out ---
        rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);
        for (int i = 0; i < sides; i++) {
            const float a0 = 2.0f * PI * static_cast<float>(i) / sides;
            const float a1 = 2.0f * PI * static_cast<float>(i + 1) / sides;
            const float c0 = std::cos(a0), s0 = std::sin(a0);
            const float c1 = std::cos(a1), s1 = std::sin(a1);

            // Wound so the OUTER surface is the front face (CCW from outside),
            // otherwise back-face culling hides the near wall and you see the
            // hollow inside. Order: bottom0 -> top0 -> top1 -> bottom1.
            rlNormal3f(c0, 0.0f, s0);
            rlVertex3f(radius * c0, -h2, radius * s0);
            rlNormal3f(c0, 0.0f, s0);
            rlVertex3f(radius * c0, h2, radius * s0);
            rlNormal3f(c1, 0.0f, s1);
            rlVertex3f(radius * c1, h2, radius * s1);
            rlNormal3f(c1, 0.0f, s1);
            rlVertex3f(radius * c1, -h2, radius * s1);
        }
        rlEnd();

        // --- top cap (+Y): a disk lifted to +h2 ---
        rlPushMatrix();
        rlTranslatef(0.0f, h2, 0.0f);
        drawDisk(radius, sides, color);
        rlPopMatrix();

        // --- bottom cap (-Y): same disk, but facing -Y and wound the other way ---
        rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlNormal3f(0.0f, -1.0f, 0.0f);
        for (int i = 0; i < sides; i++) {
            const float a0 = 2.0f * PI * static_cast<float>(i) / sides;
            const float a1 = 2.0f * PI * static_cast<float>(i + 1) / sides;

            rlVertex3f(0.0f, -h2, 0.0f);
            rlVertex3f(radius * std::cos(a1), -h2, -radius * std::sin(a1));
            rlVertex3f(radius * std::cos(a0), -h2, -radius * std::sin(a0));
        }
        rlEnd();
    }

} // namespace raylibgl::model
