#include "model/Scooby-van.hpp"

#include "model/Primitives.hpp"

#include <cmath>

#include <raymath.h>
#include <rlgl.h>

namespace raylibgl::model {

    namespace {


        constexpr Color BODY_TURQUOISE = Color{28, 181, 186, 255};
        constexpr Color BODY_SHADE = Color{15, 128, 134, 255};
        constexpr Color BODY_DARK = Color{8, 89, 94, 255};
        constexpr Color TRIM_ORANGE = Color{237, 118, 40, 255};
        constexpr Color TRIM_YELLOW = Color{247, 225, 111, 255};
        constexpr Color TRIM_BLUE = Color{60, 167, 215, 255};
        constexpr Color FLOWER_RED = Color{229, 79, 37, 255};
        constexpr Color FLOWER_ORANGE = Color{250, 191, 74, 255};
        constexpr Color METAL_GREY = Color{175, 185, 196, 255};
        constexpr Color METAL_DARK = Color{104, 114, 126, 255};
        constexpr Color RUBBER_BLACK = Color{24, 29, 35, 255};
        constexpr Color HUBCAP_BLUE = Color{88, 190, 224, 255};
        constexpr Color TAIL_RED = Color{215, 52, 48, 255};
        constexpr Color GLASS_TINT = Color{185, 232, 255, 165};
        constexpr Color GLASS_HILITE = Color{240, 250, 255, 215};
        constexpr Color GLASS_FRAME = Color{18, 36, 44, 255};
        constexpr Color SEAM_LINE = Color{18, 55, 60, 255};
        constexpr Color BLACK_TRIM = Color{14, 18, 22, 255};

        constexpr float BODY_HALF_W = 1.05f;
        constexpr float SIDE_GRAPHIC_OFFSET = 0.019f;
        constexpr float SIDE_FRAME_OFFSET = 0.016f;
        constexpr float SIDE_GLASS_OFFSET = 0.018f;
        constexpr float WINDSHIELD_FRAME_OFFSET = 0.012f;
        constexpr float WINDSHIELD_GLASS_OFFSET = 0.018f;
        constexpr float SEAM_OFFSET = 0.0205f;

        constexpr float TIRE_RADIUS = 0.41f;
        constexpr float TIRE_WIDTH = 0.34f;
        constexpr int WHEEL_SIDES = 10;
        constexpr int LIGHT_SIDES = 8;
        constexpr int SPARE_SIDES = 10;

        struct ProfilePt {
            float y;
            float z;
        };

        struct SidePt {
            float y;
            float z;
        };

        constexpr ProfilePt BODY_PROFILE[] = {
            {0.34f, -1.78f}, {0.34f, 1.92f}, {1.42f, 2.20f}, {2.50f, 1.60f}, {2.50f, -1.18f}, {1.46f, -2.02f},
        };
        constexpr int BODY_PROFILE_N = sizeof(BODY_PROFILE) / sizeof(BODY_PROFILE[0]);
        constexpr int BODY_CAP_TRIS[] = {
            0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5,
        };
        constexpr int BODY_CAP_INDEX_COUNT = sizeof(BODY_CAP_TRIS) / sizeof(BODY_CAP_TRIS[0]);

        constexpr SidePt FRONT_DOOR[] = {
            {0.52f, -1.56f}, {0.52f, -0.62f}, {2.28f, -0.62f}, {2.28f, -1.04f}, {1.48f, -1.56f},
        };

        // Inset from the door silhouette, shaped like the actual door glass instead of a rectangle.
        constexpr SidePt DOOR_WINDOW_FRAME[] = {
            {1.36f, -1.44f}, {2.18f, -1.22f}, {2.18f, -0.76f}, {1.88f, -0.70f}, {1.36f, -0.92f},
        };

        constexpr SidePt DOOR_WINDOW_GLASS[] = {
            {1.42f, -1.36f}, {2.10f, -1.18f}, {2.10f, -0.82f}, {1.84f, -0.77f}, {1.42f, -0.95f},
        };

        // Side graphic bands. These are procedural instead of a giant side decal so
        // they stay correctly oriented and wrap more faithfully.
        constexpr SidePt BELTLINE[] = {
            {1.40f, -1.74f},
            {1.48f, -1.74f},
            {1.48f, 1.74f},
            {1.40f, 1.74f},
        };

        constexpr SidePt YELLOW_WAVE[] = {
            {0.90f, -1.72f}, {1.34f, -1.72f}, {1.26f, -0.80f}, {1.12f, 0.05f},  {1.22f, 1.10f},
            {1.10f, 1.72f},  {0.86f, 1.72f},  {0.74f, 0.95f},  {0.84f, -0.20f},
        };

        constexpr SidePt BLUE_WAVE[] = {
            {0.66f, -1.70f}, {1.00f, -1.70f}, {0.94f, -0.65f}, {0.86f, 0.25f},  {0.96f, 1.22f},
            {0.88f, 1.72f},  {0.58f, 1.72f},  {0.46f, 1.00f},  {0.56f, -0.15f},
        };

        Texture2D g_windowTexture{};
        Texture2D g_logoTexture{};
        bool g_resourcesLoaded = false;

        void drawFilledCircle(Image& image, int x, int y, int radius, Color color) {
            ImageDrawCircle(&image, x, y, radius, color);
        }

        Texture2D makeWindowTexture() {
            Image image = GenImageColor(96, 64, BLANK);

            ImageDrawRectangle(&image, 0, 0, image.width, image.height, Color{125, 206, 245, 150});
            ImageDrawRectangle(&image, 0, image.height / 2, image.width, image.height / 2, Color{70, 135, 192, 150});
            ImageDrawRectangle(&image, 6, 6, image.width - 12, 8, GLASS_HILITE);
            ImageDrawLine(&image, 12, 18, 84, 14, Color{240, 248, 255, 140});
            ImageDrawLine(&image, 8, 42, 86, 28, Color{210, 240, 255, 90});
            ImageDrawLine(&image, 0, 52, 48, 63, Color{48, 95, 145, 90});

            Texture2D texture = LoadTextureFromImage(image);
            SetTextureFilter(texture, TEXTURE_FILTER_POINT);
            UnloadImage(image);
            return texture;
        }

        Image makeLogoImage() {
            Image image = GenImageColor(256, 104, BLANK);

            const int cx = 32;
            const int cy = 52;
            drawFilledCircle(image, cx - 10, cy, 11, FLOWER_ORANGE);
            drawFilledCircle(image, cx + 10, cy, 11, FLOWER_ORANGE);
            drawFilledCircle(image, cx, cy - 10, 11, FLOWER_ORANGE);
            drawFilledCircle(image, cx, cy + 10, 11, FLOWER_ORANGE);
            drawFilledCircle(image, cx, cy, 9, FLOWER_RED);

            ImageDrawText(&image, "THE", 70, 14, 18, Color{115, 44, 10, 255});
            ImageDrawText(&image, "MYSTERY", 70, 36, 26, Color{115, 44, 10, 255});
            ImageDrawText(&image, "MACHINE", 70, 68, 20, Color{115, 44, 10, 255});
            return image;
        }

        Texture2D makeLogoTexture() {
            Image image = makeLogoImage();
            Texture2D texture = LoadTextureFromImage(image);
            SetTextureFilter(texture, TEXTURE_FILTER_POINT);
            UnloadImage(image);
            return texture;
        }

        void ensureResourcesLoaded() {
            if (g_resourcesLoaded) {
                return;
            }

            g_windowTexture = makeWindowTexture();
            g_logoTexture = makeLogoTexture();
            g_resourcesLoaded = true;
        }

        Vector3 sideNormal(const ProfilePt& a, const ProfilePt& b) {
            const float dy = b.y - a.y;
            const float dz = b.z - a.z;
            const float length = std::sqrt(dy * dy + dz * dz);

            if (length <= EPSILON) {
                return {0.0f, 1.0f, 0.0f};
            }

            return {0.0f, -dz / length, dy / length};
        }

        void emitProfileVertex(float x, const ProfilePt& p) {
            rlVertex3f(x, p.y, p.z);
        }

        void drawExtrudedX(const ProfilePt* profile, int n, float halfW, const int* capTris, int capIndexCount, Color c,
                           bool wire) {
            const float L = -halfW;
            const float R = +halfW;

            if (wire) {
                rlBegin(RL_LINES);
                rlColor4ub(c.r, c.g, c.b, c.a);

                for (int i = 0; i < n; ++i) {
                    const ProfilePt& a = profile[i];
                    const ProfilePt& b = profile[(i + 1) % n];

                    emitProfileVertex(L, a);
                    emitProfileVertex(L, b);

                    emitProfileVertex(R, a);
                    emitProfileVertex(R, b);

                    emitProfileVertex(L, a);
                    emitProfileVertex(R, a);
                }

                rlEnd();
                return;
            }

            rlBegin(RL_QUADS);
            rlColor4ub(c.r, c.g, c.b, c.a);
            for (int i = 0; i < n; ++i) {
                const ProfilePt& a = profile[i];
                const ProfilePt& b = profile[(i + 1) % n];
                const Vector3 normal = sideNormal(a, b);

                rlNormal3f(normal.x, normal.y, normal.z);
                emitProfileVertex(L, a);
                emitProfileVertex(R, a);
                emitProfileVertex(R, b);
                emitProfileVertex(L, b);
            }
            rlEnd();

            rlBegin(RL_TRIANGLES);
            rlColor4ub(c.r, c.g, c.b, c.a);
            for (int i = 0; i < capIndexCount; i += 3) {
                const ProfilePt& a = profile[capTris[i]];
                const ProfilePt& b = profile[capTris[i + 1]];
                const ProfilePt& d = profile[capTris[i + 2]];

                rlNormal3f(-1.0f, 0.0f, 0.0f);
                emitProfileVertex(L, a);
                emitProfileVertex(L, b);
                emitProfileVertex(L, d);

                rlNormal3f(1.0f, 0.0f, 0.0f);
                emitProfileVertex(R, a);
                emitProfileVertex(R, d);
                emitProfileVertex(R, b);
            }
            rlEnd();
        }

        void drawQuadOutline(Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color color) {
            rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex3f(a.x, a.y, a.z);
            rlVertex3f(b.x, b.y, b.z);
            rlVertex3f(b.x, b.y, b.z);
            rlVertex3f(c.x, c.y, c.z);
            rlVertex3f(c.x, c.y, c.z);
            rlVertex3f(d.x, d.y, d.z);
            rlVertex3f(d.x, d.y, d.z);
            rlVertex3f(a.x, a.y, a.z);
            rlEnd();
        }

        void drawSolidQuad(Vector3 normal, Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color tint) {
            rlBegin(RL_QUADS);
            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            rlNormal3f(normal.x, normal.y, normal.z);
            rlVertex3f(a.x, a.y, a.z);
            rlVertex3f(b.x, b.y, b.z);
            rlVertex3f(c.x, c.y, c.z);
            rlVertex3f(d.x, d.y, d.z);
            rlEnd();
        }

        void beginAlphaTextured(Texture2D texture, Color tint, Vector3 normal) {
            rlSetBlendMode(RL_BLEND_ALPHA);
            rlEnableColorBlend();
            rlSetTexture(texture.id);
            rlBegin(RL_QUADS);
            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            rlNormal3f(normal.x, normal.y, normal.z);
        }

        void endAlphaTextured() {
            rlEnd();
            rlSetTexture(0);
        }

        void drawTexturedQuad(Texture2D texture, Vector3 normal, Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color tint,
                              bool flipU) {
            beginAlphaTextured(texture, tint, normal);
            const float u0 = flipU ? 1.0f : 0.0f;
            const float u1 = flipU ? 0.0f : 1.0f;
            rlTexCoord2f(u0, 1.0f);
            rlVertex3f(a.x, a.y, a.z);
            rlTexCoord2f(u0, 0.0f);
            rlVertex3f(b.x, b.y, b.z);
            rlTexCoord2f(u1, 0.0f);
            rlVertex3f(c.x, c.y, c.z);
            rlTexCoord2f(u1, 1.0f);
            rlVertex3f(d.x, d.y, d.z);
            endAlphaTextured();
        }

        void drawSideQuad(int sx, float xAbs, const SidePt& p0, const SidePt& p1, const SidePt& p2, const SidePt& p3,
                          Color tint, bool wire) {
            const float x = sx * xAbs;
            const Vector3 normal{static_cast<float>(sx), 0.0f, 0.0f};
            const Vector3 v0{x, p0.y, p0.z};
            const Vector3 v1{x, p1.y, p1.z};
            const Vector3 v2{x, p2.y, p2.z};
            const Vector3 v3{x, p3.y, p3.z};

            if (wire) {
                if (sx > 0) {
                    drawQuadOutline(v0, v1, v2, v3, tint);
                } else {
                    drawQuadOutline(v0, v3, v2, v1, tint);
                }
                return;
            }

            if (sx > 0) {
                drawSolidQuad(normal, v0, v1, v2, v3, tint);
            } else {
                drawSolidQuad(normal, v0, v3, v2, v1, tint);
            }
        }

        void drawSidePolygon(int sx, float xAbs, const SidePt* pts, int count, Color tint, bool wire) {
            const float x = sx * xAbs;
            const Vector3 normal{static_cast<float>(sx), 0.0f, 0.0f};

            if (wire) {
                rlBegin(RL_LINES);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);
                for (int i = 0; i < count; ++i) {
                    const SidePt& a = pts[i];
                    const SidePt& b = pts[(i + 1) % count];
                    rlVertex3f(x, a.y, a.z);
                    rlVertex3f(x, b.y, b.z);
                }
                rlEnd();
                return;
            }

            rlBegin(RL_TRIANGLES);
            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            for (int i = 1; i < count - 1; ++i) {
                if (sx > 0) {
                    rlNormal3f(normal.x, normal.y, normal.z);
                    rlVertex3f(x, pts[0].y, pts[0].z);
                    rlVertex3f(x, pts[i].y, pts[i].z);
                    rlVertex3f(x, pts[i + 1].y, pts[i + 1].z);
                } else {
                    rlNormal3f(normal.x, normal.y, normal.z);
                    rlVertex3f(x, pts[0].y, pts[0].z);
                    rlVertex3f(x, pts[i + 1].y, pts[i + 1].z);
                    rlVertex3f(x, pts[i].y, pts[i].z);
                }
            }
            rlEnd();
        }

        void drawSideLogo(int sx, bool wire) {
            ensureResourcesLoaded();
            const Texture2D logo = g_logoTexture;
            const float x = sx * (BODY_HALF_W + SIDE_GRAPHIC_OFFSET);
            const Vector3 normal{static_cast<float>(sx), 0.0f, 0.0f};
            const Vector3 a{x, 0.92f, -0.10f};
            const Vector3 b{x, 1.76f, -0.10f};
            const Vector3 c{x, 1.76f, 1.50f};
            const Vector3 d{x, 0.92f, 1.50f};

            if (wire) {
                if (sx > 0) {
                    drawQuadOutline(a, b, c, d, WHITE);
                } else {
                    drawQuadOutline(a, d, c, b, WHITE);
                }
                return;
            }

            if (sx > 0) {
                drawTexturedQuad(logo, normal, a, b, c, d, WHITE, false);
            } else {
                drawTexturedQuad(logo, normal, a, d, c, b, WHITE, true);
            }
        }

        void drawSideLogos(bool wire) {
            for (int sx = -1; sx <= 1; sx += 2) {
                drawSideLogo(sx, wire);
            }
        }

        void drawDoorSeams() {
            for (int sx = -1; sx <= 1; sx += 2) {
                const float x = sx * (BODY_HALF_W + SEAM_OFFSET);
                rlBegin(RL_LINES);
                rlColor4ub(SEAM_LINE.r, SEAM_LINE.g, SEAM_LINE.b, SEAM_LINE.a);

                const int count = static_cast<int>(sizeof(FRONT_DOOR) / sizeof(FRONT_DOOR[0]));
                for (int i = 0; i < count; ++i) {
                    const SidePt& a = FRONT_DOOR[i];
                    const SidePt& b = FRONT_DOOR[(i + 1) % count];
                    rlVertex3f(x, a.y, a.z);
                    rlVertex3f(x, b.y, b.z);
                }

                // Sliding-door seam and rear body seam.
                rlVertex3f(x, 0.58f, -0.12f);
                rlVertex3f(x, 2.16f, -0.12f);
                rlVertex3f(x, 0.58f, 0.96f);
                rlVertex3f(x, 2.04f, 0.96f);

                // Handles.
                rlVertex3f(x, 1.28f, -0.92f);
                rlVertex3f(x, 1.28f, -0.68f);
                rlVertex3f(x, 1.28f, 0.16f);
                rlVertex3f(x, 1.28f, 0.42f);
                rlEnd();
            }
        }

        void drawWrappedTrim(bool wire) {
            // Side beltline plus simplified front/rear continuation.
            for (int sx = -1; sx <= 1; sx += 2) {
                drawSidePolygon(sx, BODY_HALF_W + SIDE_GRAPHIC_OFFSET, BELTLINE,
                                static_cast<int>(sizeof(BELTLINE) / sizeof(BELTLINE[0])), TRIM_ORANGE, wire);
            }

            // Front continuation on the slanted front face.
            const float zTop = -1.62f;
            const float zBottom = -1.70f;
            const Vector3 fa{-0.92f, 1.40f, zBottom};
            const Vector3 fb{-0.92f, 1.48f, zTop};
            const Vector3 fc{0.92f, 1.48f, zTop};
            const Vector3 fd{0.92f, 1.40f, zBottom};
            if (wire) {
                drawQuadOutline(fa, fb, fc, fd, TRIM_ORANGE);
            } else {
                drawSolidQuad(Vector3{0.0f, 0.62f, -0.78f}, fa, fb, fc, fd, TRIM_ORANGE);
            }

            const Vector3 ra{-0.92f, 1.40f, 2.04f};
            const Vector3 rb{-0.92f, 1.48f, 2.04f};
            const Vector3 rc{0.92f, 1.48f, 2.04f};
            const Vector3 rd{0.92f, 1.40f, 2.04f};
            if (wire) {
                drawQuadOutline(ra, rb, rc, rd, TRIM_ORANGE);
            } else {
                drawSolidQuad(Vector3{0.0f, 0.0f, 1.0f}, ra, rb, rc, rd, TRIM_ORANGE);
            }
        }

        void drawWaveGraphics(bool wire) {
            for (int sx = -1; sx <= 1; sx += 2) {
                drawSidePolygon(sx, BODY_HALF_W + SIDE_GRAPHIC_OFFSET, YELLOW_WAVE,
                                static_cast<int>(sizeof(YELLOW_WAVE) / sizeof(YELLOW_WAVE[0])), TRIM_YELLOW, wire);
                drawSidePolygon(sx, BODY_HALF_W + SIDE_GRAPHIC_OFFSET, BLUE_WAVE,
                                static_cast<int>(sizeof(BLUE_WAVE) / sizeof(BLUE_WAVE[0])), TRIM_BLUE, wire);
            }

            // Small continuation hints on the front and back so the "equator" treatment wraps visually.
            rlPushMatrix();
            rlTranslatef(0.0f, 0.92f, -1.92f);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, Vector3{1.96f, 0.20f, 0.05f}, TRIM_YELLOW, wire);
            rlPopMatrix();
            rlPushMatrix();
            rlTranslatef(0.0f, 0.68f, -1.94f);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, Vector3{1.96f, 0.16f, 0.05f}, TRIM_BLUE, wire);
            rlPopMatrix();

            rlPushMatrix();
            rlTranslatef(0.0f, 0.92f, 2.02f);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, Vector3{1.88f, 0.20f, 0.05f}, TRIM_YELLOW, wire);
            rlPopMatrix();
            rlPushMatrix();
            rlTranslatef(0.0f, 0.68f, 2.00f);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, Vector3{1.88f, 0.16f, 0.05f}, TRIM_BLUE, wire);
            rlPopMatrix();
        }

        void drawFrontWindshield(bool wire) {
            ensureResourcesLoaded();

            constexpr Vector3 normal{0.0f, 0.62f, -0.78f};
            const Vector3 fa{-0.76f, 2.26f, -1.29f};
            const Vector3 fb{0.76f, 2.26f, -1.29f};
            const Vector3 fc{0.66f, 1.50f, -1.93f};
            const Vector3 fd{-0.66f, 1.50f, -1.93f};
            const Vector3 ga{-0.68f, 2.18f, -1.31f};
            const Vector3 gb{0.68f, 2.18f, -1.31f};
            const Vector3 gc{0.58f, 1.56f, -1.86f};
            const Vector3 gd{-0.58f, 1.56f, -1.86f};

            if (wire) {
                drawQuadOutline(fa, fb, fc, fd, GLASS_FRAME);
                drawQuadOutline(ga, gb, gc, gd, GLASS_TINT);
                return;
            }

            drawSolidQuad(normal, fa, fb, fc, fd, GLASS_FRAME);
            drawTexturedQuad(g_windowTexture, normal, ga, gb, gc, gd, GLASS_TINT, false);
        }

        void drawDoorWindows(bool wire) {
            ensureResourcesLoaded();
            for (int sx = -1; sx <= 1; sx += 2) {
                drawSidePolygon(sx, BODY_HALF_W + SIDE_FRAME_OFFSET, DOOR_WINDOW_FRAME,
                                static_cast<int>(sizeof(DOOR_WINDOW_FRAME) / sizeof(DOOR_WINDOW_FRAME[0])), GLASS_FRAME,
                                wire);

                // Use the bounding quad of the shaped glass polygon as a texture carrier.
                // The darker frame above makes it read as inset even though the model remains procedural.
                const float x = sx * (BODY_HALF_W + SIDE_GLASS_OFFSET);
                const Vector3 normal{static_cast<float>(sx), 0.0f, 0.0f};
                const Vector3 a{x, DOOR_WINDOW_GLASS[0].y, DOOR_WINDOW_GLASS[0].z};
                const Vector3 b{x, DOOR_WINDOW_GLASS[1].y, DOOR_WINDOW_GLASS[1].z};
                const Vector3 c{x, DOOR_WINDOW_GLASS[2].y, DOOR_WINDOW_GLASS[2].z};
                const Vector3 d{x, DOOR_WINDOW_GLASS[4].y, DOOR_WINDOW_GLASS[4].z};
                if (wire) {
                    if (sx > 0) {
                        drawQuadOutline(a, b, c, d, GLASS_TINT);
                    } else {
                        drawQuadOutline(a, d, c, b, GLASS_TINT);
                    }
                } else if (sx > 0) {
                    drawTexturedQuad(g_windowTexture, normal, a, b, c, d, GLASS_TINT, false);
                } else {
                    drawTexturedQuad(g_windowTexture, normal, a, d, c, b, GLASS_TINT, false);
                }
            }
        }

        void drawBumpers(bool wire) {
            constexpr Vector3 FRONT_BUMPER = {2.16f, 0.32f, 0.24f};
            constexpr Vector3 REAR_BUMPER = {2.10f, 0.30f, 0.22f};
            constexpr float BUMPER_Y = 0.50f;

            rlPushMatrix();
            rlTranslatef(0.0f, BUMPER_Y, -1.90f);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, FRONT_BUMPER, METAL_GREY, wire);
            rlPopMatrix();

            rlPushMatrix();
            rlTranslatef(0.0f, BUMPER_Y, 2.02f);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, REAR_BUMPER, METAL_GREY, wire);
            rlPopMatrix();
        }

        void drawRoofBars(bool wire) {
            // Only the two width-wise cross bars, matching the reference.
            constexpr Vector3 CROSS_BAR = {1.70f, 0.10f, 0.10f};
            constexpr float Y = 2.58f;
            constexpr float Z_VALUES[] = {-0.18f, 0.92f};
            for (float z: Z_VALUES) {
                rlPushMatrix();
                rlTranslatef(0.0f, Y, z);
                drawBox(Vector3{0.0f, 0.0f, 0.0f}, CROSS_BAR, METAL_GREY, wire);
                rlPopMatrix();
            }
        }

        void drawFrontDetails(bool wire) {
            rlPushMatrix();
            rlTranslatef(0.0f, 0.98f, -1.97f);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, Vector3{1.00f, 0.34f, 0.06f}, METAL_DARK, wire);
            rlPopMatrix();

            rlPushMatrix();
            rlTranslatef(0.0f, 0.78f, -1.99f);
            drawBox(Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.62f, 0.16f, 0.04f}, Color{240, 241, 232, 255}, wire);
            rlPopMatrix();
        }

        void drawLightsAndMarkers(bool wire) {
            constexpr float LIGHT_X = 0.66f;
            constexpr float LIGHT_Y = 1.02f;
            constexpr float LIGHT_Z = -1.94f;
            for (int sx = -1; sx <= 1; sx += 2) {
                rlPushMatrix();
                rlTranslatef(sx * LIGHT_X, LIGHT_Y, LIGHT_Z);
                drawLight(wire);
                rlPopMatrix();
            }

            constexpr Vector3 TAIL_SIZE = {0.28f, 0.20f, 0.08f};
            constexpr float TAIL_X = 0.68f;
            constexpr float TAIL_Y = 0.90f;
            constexpr float TAIL_Z = 2.12f;
            for (int sx = -1; sx <= 1; sx += 2) {
                rlPushMatrix();
                rlTranslatef(sx * TAIL_X, TAIL_Y, TAIL_Z);
                drawBox(Vector3{0.0f, 0.0f, 0.0f}, TAIL_SIZE, TAIL_RED, wire);
                rlPopMatrix();
            }
        }

        void drawMirrors(bool wire) {
            constexpr Vector3 MIRROR_SIZE = {0.20f, 0.28f, 0.08f};
            constexpr Vector3 ARM_SIZE = {0.06f, 0.10f, 0.12f};
            constexpr float MIRROR_X = BODY_HALF_W + 0.12f;
            constexpr float MIRROR_Y = 1.52f;
            constexpr float MIRROR_Z = -1.20f;
            for (int sx = -1; sx <= 1; sx += 2) {
                rlPushMatrix();
                rlTranslatef(sx * MIRROR_X, MIRROR_Y, MIRROR_Z);
                drawBox(Vector3{0.0f, 0.0f, 0.0f}, MIRROR_SIZE, BODY_TURQUOISE, wire);
                rlPopMatrix();

                rlPushMatrix();
                rlTranslatef(sx * (BODY_HALF_W + 0.06f), MIRROR_Y - 0.08f, MIRROR_Z + 0.08f);
                drawBox(Vector3{0.0f, 0.0f, 0.0f}, ARM_SIZE, METAL_DARK, wire);
                rlPopMatrix();
            }
        }

        void drawSpareWheel(bool wire) {
            constexpr float SPARE_R = 0.44f;
            constexpr float SPARE_DEPTH = 0.18f;

            rlPushMatrix();
            rlTranslatef(0.0f, 1.28f, -1.98f);
            rlRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            drawCylinder(Vector3{0.0f, 0.0f, 0.0f}, SPARE_R, SPARE_DEPTH, RUBBER_BLACK, wire, SPARE_SIDES);
            drawCylinder(Vector3{0.0f, 0.01f, 0.0f}, SPARE_R * 0.58f, SPARE_DEPTH + 0.02f, HUBCAP_BLUE, wire,
                         SPARE_SIDES);
            rlPopMatrix();
        }

        void drawWheelSet(bool wire) {
            constexpr float WHEEL_X = 0.96f;
            constexpr float WHEEL_Y = 0.33f;
            constexpr float WHEEL_Z[] = {-0.88f, 1.02f};
            for (int sx = -1; sx <= 1; sx += 2) {
                for (float wz: WHEEL_Z) {
                    rlPushMatrix();
                    rlTranslatef(sx * WHEEL_X, WHEEL_Y, wz);
                    drawWheel(wire);
                    rlPopMatrix();
                }
            }
        }

    } // namespace

    void LoadCarResources() {
        ensureResourcesLoaded();
    }

    void UnloadCarResources() {
        if (!g_resourcesLoaded) {
            return;
        }

        if (g_windowTexture.id != 0) {
            UnloadTexture(g_windowTexture);
        }
        if (g_logoTexture.id != 0) {
            UnloadTexture(g_logoTexture);
        }

        g_windowTexture = {};
        g_logoTexture = {};
        g_resourcesLoaded = false;
    }

    void drawChassis(bool wire) {
        drawExtrudedX(BODY_PROFILE, BODY_PROFILE_N, BODY_HALF_W, BODY_CAP_TRIS, BODY_CAP_INDEX_COUNT, BODY_TURQUOISE,
                      wire);
    }

    void drawWheel(bool wire) {
        rlPushMatrix();
        rlRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
        drawCylinder(Vector3{0.0f, 0.0f, 0.0f}, TIRE_RADIUS, TIRE_WIDTH, RUBBER_BLACK, wire, WHEEL_SIDES);
        drawCylinder(Vector3{0.0f, -0.11f, 0.0f}, TIRE_RADIUS * 0.54f, 0.04f, HUBCAP_BLUE, wire, WHEEL_SIDES);
        drawCylinder(Vector3{0.0f, 0.11f, 0.0f}, TIRE_RADIUS * 0.54f, 0.04f, HUBCAP_BLUE, wire, WHEEL_SIDES);
        rlPopMatrix();
    }

    void drawLight(bool wire) {
        constexpr float r = 0.23f;
        constexpr float depth = 0.10f;

        rlPushMatrix();
        rlRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        drawCylinder(Vector3{0.0f, 0.0f, 0.0f}, r, depth, TRIM_YELLOW, wire, LIGHT_SIDES);
        rlPopMatrix();
    }

    void drawCar(bool wire) {
        ensureResourcesLoaded();

        rlPushMatrix();
        {
            rlTranslatef(0.0f, -1.24f, 0.0f);

            drawChassis(wire);
            drawWrappedTrim(wire);
            drawWaveGraphics(wire);
            drawDoorSeams();
            drawBumpers(wire);
            drawRoofBars(wire);
            drawFrontDetails(wire);
            drawLightsAndMarkers(wire);
            drawWheelSet(wire);
            drawSpareWheel(wire);
            drawMirrors(wire);
            drawSideLogos(wire);

            // Transparent-ish glass last.
            drawFrontWindshield(wire);
            drawDoorWindows(wire);
        }
        rlPopMatrix();
    }

} // namespace raylibgl::model
