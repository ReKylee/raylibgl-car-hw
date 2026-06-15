#include "app/Application.hpp"

#include "model/Primitives.hpp"

#include "raymath.h"
#include "rlgl.h"

#include <utility>

namespace raylibgl::app {

    namespace {

        constexpr int TARGET_FPS = 60;

    } // namespace

    Application::Application(int width, int height, const char *title) :
        m_width(width), m_height(height), m_title(title) {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);

        InitWindow(m_width, m_height, m_title);
        SetExitKey(KEY_F10);
        SetTargetFPS(TARGET_FPS);

        // Assignment requirement: back-face culling stays on at all times.
        // raylib enables this by default with counter-clockwise = front face;
        // our primitives are wound to match, so back faces are correctly hidden.
        rlEnableBackfaceCulling();


        // Do NOT disable the cursor for an orbit camera.
        // Orbit controls use normal mouse movement and middle mouse dragging.
        //
        // DisableCursor();
    }

    Application::~Application() { CloseWindow(); }

    void Application::run() {
        while (!WindowShouldClose()) {
            update();
            render();
        }
    }

    void Application::update() {
        m_width = GetScreenWidth();
        m_height = GetScreenHeight();

        // debug::Update(m_debugState);

        if (IsKeyPressed(KEY_A)) {
            m_showAxes = !m_showAxes;
        }

        m_camera.Update(m_width, m_height);
    }

    void Application::render() {
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(m_camera.GetCamera());

        drawSceneRlgl();

        EndMode3D();

        // debug::DrawOverlay(m_debugState);

        EndDrawing();
    }

    void Application::drawSceneRlgl() {
        DrawGrid(20, 1.0f);

        // Apply the virtual-trackball rotation to the whole model, about the
        // origin (per the assignment: "rotate the world about the origin using
        // the ModelView matrix"). The axes rotate with it, so they double as an
        // orientation gizmo.
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(m_camera.GetRotation()));

        if (m_showAxes) {
            model::drawAxes(2.5f);
        }

        // Super-basic "car" using raylib's built-in primitives.
        // Body: red cube centered at the origin.
        DrawCubeV(Vector3{0.0f, 0.0f, 0.0f}, Vector3{2.0f, 2.0f, 2.0f}, RED);

        // 4 green wheels. DrawCylinderEx takes the two end-cap centers, so the
        // axle runs left-right (X) just by offsetting in X - no rotation needed.
        const float wheelRadius = 0.4f;
        const float wheelHalfWidth = 0.25f;
        for (int sx = -1; sx <= 1; sx += 2) {     // -1 = left,  +1 = right
            for (int sz = -1; sz <= 1; sz += 2) { // -1 = front, +1 = back
                const float x = sx * 1.0f, y = -0.8f, z = sz * 0.6f;
                DrawCylinderEx(Vector3{x - wheelHalfWidth, y, z},
                               Vector3{x + wheelHalfWidth, y, z},
                               wheelRadius, wheelRadius, 16, GREEN);
            }
        }

        // 2 yellow lights on the +Z face. Axle runs front-back (Z).
        const float lightRadius = 0.2f;
        const float lightHalfDepth = 0.2f;
        for (int sx = -1; sx <= 1; sx += 2) {
            const float x = sx * 0.5f, y = 0.15f, z = 1.0f;
            DrawCylinderEx(Vector3{x, y, z - lightHalfDepth},
                           Vector3{x, y, z + lightHalfDepth},
                           lightRadius, lightRadius, 16, YELLOW);
        }

        rlPopMatrix();
    }

} // namespace raylibgl::app
