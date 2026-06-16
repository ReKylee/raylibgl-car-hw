#include "model/Primitives.hpp"

#include <cmath>

#include <rlgl.h>

namespace raylibgl::model {

    namespace {

        constexpr float TWO_PI = 6.28318530717958647692f;

        Vector3 cylinderPoint(Vector3 center, float radius, float y, float angle) {
            return {
                center.x + std::sin(angle) * radius,
                center.y + y,
                center.z + std::cos(angle) * radius,
            };
        }

        Vector3 cylinderNormal(float angle) {
            return {std::sin(angle), 0.0f, std::cos(angle)};
        }

        void emitVertex(Vector3 normal, Vector3 point) {
            rlNormal3f(normal.x, normal.y, normal.z);
            rlVertex3f(point.x, point.y, point.z);
        }

    } // namespace

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

    void drawCylinder(Vector3 center, float radius, float height, Color color, bool wire, int sides) {
        if (sides < 3) {
            sides = 3;
        }

        const float bottomY = -height * 0.5f;
        const float topY = height * 0.5f;
        const float step = TWO_PI / static_cast<float>(sides);

        if (wire) {
            rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < sides; ++i) {
                const float a0 = step * static_cast<float>(i);
                const float a1 = step * static_cast<float>(i + 1);

                const Vector3 b0 = cylinderPoint(center, radius, bottomY, a0);
                const Vector3 b1 = cylinderPoint(center, radius, bottomY, a1);
                const Vector3 t0 = cylinderPoint(center, radius, topY, a0);
                const Vector3 t1 = cylinderPoint(center, radius, topY, a1);

                rlVertex3f(b0.x, b0.y, b0.z);
                rlVertex3f(b1.x, b1.y, b1.z);

                rlVertex3f(t0.x, t0.y, t0.z);
                rlVertex3f(t1.x, t1.y, t1.z);

                rlVertex3f(b0.x, b0.y, b0.z);
                rlVertex3f(t0.x, t0.y, t0.z);
            }

            rlEnd();
            return;
        }

        rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < sides; ++i) {
            const float a0 = step * static_cast<float>(i);
            const float a1 = step * static_cast<float>(i + 1);
            const float am = (a0 + a1) * 0.5f;

            const Vector3 b0 = cylinderPoint(center, radius, bottomY, a0);
            const Vector3 b1 = cylinderPoint(center, radius, bottomY, a1);
            const Vector3 t0 = cylinderPoint(center, radius, topY, a0);
            const Vector3 t1 = cylinderPoint(center, radius, topY, a1);

            // Same normal for both triangles in this side segment: intentionally faceted.
            const Vector3 sideN = cylinderNormal(am);

            emitVertex(sideN, b0);
            emitVertex(sideN, b1);
            emitVertex(sideN, t1);

            emitVertex(sideN, b0);
            emitVertex(sideN, t1);
            emitVertex(sideN, t0);

            // Top disk.
            emitVertex({0.0f, 1.0f, 0.0f}, {center.x, center.y + topY, center.z});
            emitVertex({0.0f, 1.0f, 0.0f}, t0);
            emitVertex({0.0f, 1.0f, 0.0f}, t1);

            // Bottom disk.
            emitVertex({0.0f, -1.0f, 0.0f}, {center.x, center.y + bottomY, center.z});
            emitVertex({0.0f, -1.0f, 0.0f}, b1);
            emitVertex({0.0f, -1.0f, 0.0f}, b0);
        }

        rlEnd();
    }

} // namespace raylibgl::model
