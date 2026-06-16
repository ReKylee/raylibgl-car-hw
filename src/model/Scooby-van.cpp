#include "model/Scooby-van.hpp"

#include "model/Primitives.hpp"

#include <rlgl.h>

namespace raylibgl::model {

    namespace {

        // The Mystery Machine's signature turquoise body.
        constexpr Color BODY_TURQUOISE = Color{60, 200, 175, 255};

        // Tires: near-black (pure black would vanish against the black
        // background; this still reads as black but stays visible unlit).
        constexpr Color TIRE_BLACK = Color{28, 28, 32, 255};

        // Tire dimensions (car-parts.md): r ~ 0.41, width ~ 0.37, axle along X.
        constexpr float TIRE_RADIUS = 0.41f;
        constexpr float TIRE_HALF_W = 0.37f / 2.0f;

        // Metal-grey trim: roof rack slats + front/back bumpers.
        constexpr Color METAL_GREY = Color{172, 175, 182, 255};

        // Door panels: a slightly darker turquoise than the body.
        constexpr Color DOOR_TURQUOISE = Color{45, 155, 135, 255};

        // A point on the body's side silhouette, in the Y-Z plane (front = -Z).
        struct ProfilePt {
            float y;
            float z;
        };

        // Body side profile: a "long hexagon" (top trapezoid + bottom trapezoid
        // sharing the widest waistline edge). Listed CCW as seen from the -X
        // side, so the extrusion below gets outward-facing windings for free.
        //
        //        P4 ___________ P3       <- roof (top, shortest in Z)
        //         /             \
        //     P5 |               | P2    <- waistline (widest in Z)
        //         \             /
        //        P0 ----------- P1        <- floor (bottom)
        //   front (-Z)        back (+Z)
        constexpr ProfilePt BODY_PROFILE[] = {
            {0.35f, -1.70f}, // P0 bottom-front
            {0.35f, 1.85f},  // P1 bottom-back
            {1.45f, 2.15f},  // P2 waist-back   (widest)
            {2.45f, 1.55f},  // P3 top-back
            {2.45f, -1.15f}, // P4 top-front
            {1.45f, -1.95f}, // P5 waist-front  (widest)
        };
        constexpr int BODY_PROFILE_N = sizeof(BODY_PROFILE) / sizeof(BODY_PROFILE[0]);

        // Half the body width along X (the extrusion runs -halfW .. +halfW).
        constexpr float BODY_HALF_W = 1.05f;

        // Door: a trapezoid (not a rectangle) — the front edge slants back as it
        // rises, following the body. Same Y-Z profile convention as the body
        // (CCW from -X); extruded as a thin slab laid on the side surface.
        constexpr ProfilePt DOOR_PROFILE[] = {
            {0.50f, -1.05f}, // bottom-front
            {0.50f, -0.10f}, // bottom-back
            {1.90f, -0.10f}, // top-back  (vertical back edge)
            {1.90f, -1.45f}, // top-front (front edge slants forward going up)
        };
        constexpr int DOOR_PROFILE_N = sizeof(DOOR_PROFILE) / sizeof(DOOR_PROFILE[0]);
        constexpr float DOOR_HALF_THICK = 0.04f; // slab thickness along X

        // Signed area of edge (b - a) about point p, in the Z-Y plane. >0 / <0
        // tell which side of edge ab the point p lies on.
        float edgeSide(const ProfilePt &p, const ProfilePt &a, const ProfilePt &b) {
            return (p.z - b.z) * (a.y - b.y) - (a.z - b.z) * (p.y - b.y);
        }

        bool pointInTri(const ProfilePt &p, const ProfilePt &a, const ProfilePt &b,
                        const ProfilePt &c) {
            const float d1 = edgeSide(p, a, b);
            const float d2 = edgeSide(p, b, c);
            const float d3 = edgeSide(p, c, a);
            const bool hasNeg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
            const bool hasPos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);
            return !(hasNeg && hasPos);
        }

        // Ear-clipping triangulation of a simple polygon (the Y-Z profile),
        // handling concave shapes. Writes index triples into `tris` (capacity
        // >= 3*(n-2)) preserving the polygon's winding; returns the index count.
        int triangulate(const ProfilePt *p, int n, int *tris) {
            if (n < 3) {
                return 0;
            }
            // Polygon orientation (signed area, z as x-axis).
            float area = 0.0f;
            for (int i = 0; i < n; ++i) {
                const ProfilePt &a = p[i];
                const ProfilePt &b = p[(i + 1) % n];
                area += a.z * b.y - b.z * a.y;
            }
            const bool ccw = area > 0.0f;

            int idx[32];
            for (int i = 0; i < n; ++i) {
                idx[i] = i;
            }
            int m = n;
            int t = 0;
            int guard = 0;
            while (m > 3 && guard++ < 256) {
                bool clipped = false;
                for (int i = 0; i < m; ++i) {
                    const int i0 = idx[(i + m - 1) % m];
                    const int i1 = idx[i];
                    const int i2 = idx[(i + 1) % m];
                    const ProfilePt &a = p[i0];
                    const ProfilePt &b = p[i1];
                    const ProfilePt &cc = p[i2];
                    // Convex corner? (turn direction must match orientation.)
                    const float cross = (b.z - a.z) * (cc.y - a.y) - (b.y - a.y) * (cc.z - a.z);
                    const bool convex = ccw ? (cross > 0.0f) : (cross < 0.0f);
                    if (!convex) {
                        continue;
                    }
                    // Ear? (no other vertex falls inside triangle a-b-cc.)
                    bool ear = true;
                    for (int j = 0; j < m; ++j) {
                        const int vj = idx[j];
                        if (vj == i0 || vj == i1 || vj == i2) {
                            continue;
                        }
                        if (pointInTri(p[vj], a, b, cc)) {
                            ear = false;
                            break;
                        }
                    }
                    if (!ear) {
                        continue;
                    }
                    tris[t++] = i0;
                    tris[t++] = i1;
                    tris[t++] = i2;
                    for (int k = i; k < m - 1; ++k) {
                        idx[k] = idx[k + 1];
                    }
                    --m;
                    clipped = true;
                    break;
                }
                if (!clipped) {
                    break; // degenerate; bail rather than spin
                }
            }
            if (m == 3) {
                tris[t++] = idx[0];
                tris[t++] = idx[1];
                tris[t++] = idx[2];
            }
            return t;
        }

        // Draw a solid whose side silhouette is `profile` (points in the Y-Z
        // plane, ordered CCW as seen from -X), extruded along X over
        // [-halfW, +halfW]. Filled, or wireframe edges when `wire` is true.
        void drawExtrudedX(const ProfilePt *profile, int n, float halfW, Color c, bool wire) {
            const float L = -halfW; // -X face
            const float R = +halfW; // +X face

            if (wire) {
                rlBegin(RL_LINES);
                rlColor4ub(c.r, c.g, c.b, c.a);
                for (int i = 0; i < n; ++i) {
                    const ProfilePt &a = profile[i];
                    const ProfilePt &b = profile[(i + 1) % n];
                    // Silhouette edge on each side, plus the cross edge joining
                    // the two sides at this vertex.
                    rlVertex3f(L, a.y, a.z);
                    rlVertex3f(L, b.y, b.z);
                    rlVertex3f(R, a.y, a.z);
                    rlVertex3f(R, b.y, b.z);
                    rlVertex3f(L, a.y, a.z);
                    rlVertex3f(R, a.y, a.z);
                }
                rlEnd();
                return;
            }

            // Side faces: one quad per profile edge A->B, wound CCW (outward)
            // as [A_left, A_right, B_right, B_left].
            rlBegin(RL_QUADS);
            rlColor4ub(c.r, c.g, c.b, c.a);
            for (int i = 0; i < n; ++i) {
                const ProfilePt &a = profile[i];
                const ProfilePt &b = profile[(i + 1) % n];
                rlVertex3f(L, a.y, a.z);
                rlVertex3f(R, a.y, a.z);
                rlVertex3f(R, b.y, b.z);
                rlVertex3f(L, b.y, b.z);
            }
            rlEnd();

            // End caps, triangulated (ear clipping, so concave profiles work).
            // -X cap keeps the profile winding (outward -X); +X cap is reversed.
            int tris[3 * 32];
            const int tn = triangulate(profile, n, tris);
            rlBegin(RL_TRIANGLES);
            rlColor4ub(c.r, c.g, c.b, c.a);
            for (int i = 0; i < tn; i += 3) {
                const ProfilePt &a = profile[tris[i]];
                const ProfilePt &b = profile[tris[i + 1]];
                const ProfilePt &d = profile[tris[i + 2]];
                // -X cap (outward normal -X)
                rlVertex3f(L, a.y, a.z);
                rlVertex3f(L, b.y, b.z);
                rlVertex3f(L, d.y, d.z);
                // +X cap (outward normal +X) — reversed winding
                rlVertex3f(R, a.y, a.z);
                rlVertex3f(R, d.y, d.z);
                rlVertex3f(R, b.y, b.z);
            }
            rlEnd();
        }

    } // namespace

    void drawChassis(bool wire) {
        // One tapered solid: a hexagonal side profile extruded across X. The 6
        // profile edges give the floor, the bent front pair (grille +
        // windshield), the roof, and the bent back pair; the hexagon end-caps
        // are the left/right body sides.
        drawExtrudedX(BODY_PROFILE, BODY_PROFILE_N, BODY_HALF_W, BODY_TURQUOISE, wire);
    }

    void drawWheel(bool wire) {
        // Cylinder spanning the axle (X), placed by its two end-cap centers.
        drawCylinder(Vector3{-TIRE_HALF_W, 0.0f, 0.0f}, Vector3{TIRE_HALF_W, 0.0f, 0.0f},
                     TIRE_RADIUS, TIRE_BLACK, wire);
    }

    void drawCar(bool wire) {
        rlPushMatrix();
        // Re-center: car-parts.md puts the visual center at (0, 1.24, 0) with
        // the wheels on the ground. Shift the whole model down so that center
        // lands on the origin, where the trackball rotates.
        rlTranslatef(0.0f, -1.24f, 0.0f);

        drawChassis(wire);

        // 4 tires (car-parts.md): x = +/-0.80, y = 0.33; front z = -0.90,
        // rear z = +1.00. Model one wheel, place it at each corner.
        // constexpr float WHEEL_X = 0.80f;
        constexpr float WHEEL_X = 0.95f;
        constexpr float WHEEL_Y = 0.33f;
        constexpr float WHEEL_Z[] = {-0.90f, 1.00f}; // front, rear
        for (int sx = -1; sx <= 1; sx += 2) {        // -1 = left, +1 = right
            for (float wz : WHEEL_Z) {
                rlPushMatrix();
                rlTranslatef(sx * WHEEL_X, WHEEL_Y, wz);
                drawWheel(wire);
                rlPopMatrix();
            }
        }

        // 2 metal-grey roof boards: thin boxes spanning the roof width (X),
        // sitting on the roof top (y = 2.45), spaced along Z. Model one, place
        // it at each Z slot.
        constexpr Vector3 BOARD_SIZE = {1.70f, 0.15f, 0.15f};
        // constexpr Vector3 BOARD_SIZE = {1.70f, 0.08f, 0.20f};
        constexpr float ROOF_TOP_Y = 2.45f;
        constexpr float BOARD_Z[] = {-0.35f, 0.75f};
        for (float bz : BOARD_Z) {
            rlPushMatrix();
            rlTranslatef(0.0f, ROOF_TOP_Y + BOARD_SIZE.y * 0.5f, bz);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, BOARD_SIZE, METAL_GREY, wire);
            rlPopMatrix();
        }

        // 2 metal-grey bumpers: boxes spanning the width (X), low on the body,
        // protruding at the front (-Z) and back (+Z). Model one, place at each
        // end.
        constexpr Vector3 BUMPER_SIZE = {2.25f, 0.50f, 0.30f};
        // constexpr Vector3 BUMPER_SIZE = {2.05f, 0.22f, 0.30f};
        constexpr float BUMPER_Y = 0.50f;
        constexpr float BUMPER_Z[] = {-1.80f, 1.90f}; // front, back
        for (float bz : BUMPER_Z) {
            rlPushMatrix();
            rlTranslatef(0.0f, BUMPER_Y, bz);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, BUMPER_SIZE, METAL_GREY, wire);
            rlPopMatrix();
        }

        // 2 doors: slightly darker turquoise trapezoid panels on each side,
        // toward the front. The trapezoid profile is extruded into a thin slab
        // and laid on the side surface (x = +/-BODY_HALF_W); modeled once and
        // mirrored left/right.
        for (int sx = -1; sx <= 1; sx += 2) {
            rlPushMatrix();
            rlTranslatef(sx * BODY_HALF_W, 0.0f, 0.0f);
            drawExtrudedX(DOOR_PROFILE, DOOR_PROFILE_N, DOOR_HALF_THICK, DOOR_TURQUOISE, wire);
            rlPopMatrix();
        }

        rlPopMatrix();
    }

} // namespace raylibgl::model
