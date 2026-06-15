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

        // Super-basic "car": red box body...
        model::drawBox({2.0f, 2.0f, 2.0f}, RED);

        // ...plus 4 green wheels. One cylinder template, placed at the 4 corners
        // (left/right = +/-X, front/back = +/-Z) with a -Z = front convention.
        // Each wheel's axis runs left-right (X), so we rotate the Y-axis cylinder
        // 90 degrees about Z to lay it down. This is the matrix-stack + symmetry
        // pattern the real car will use.
        for (int sx = -1; sx <= 1; sx += 2) {     // -1 = left,  +1 = right
            for (int sz = -1; sz <= 1; sz += 2) { // -1 = front, +1 = back
                rlPushMatrix();
                rlTranslatef(sx * 1.0f, -0.8f, sz * 0.6f);
                rlRotatef(90.0f, 0.0f, 0.0f, 1.0f); // Y-axis cylinder -> X axis
                model::drawCylinder(0.4f, 0.5f, 16, GREEN);
                rlPopMatrix();
            }
        }

        // ...and 2 yellow lights on the front (-Z). Their axis runs front-back
        // (Z), so we rotate the Y-axis cylinder 90 degrees about X. Mirror pair
        // in X, poking out of the front face.
        for (int sx = -1; sx <= 1; sx += 2) {
            rlPushMatrix();
            rlTranslatef(sx * 0.5f, 0.15f, 1.0f);
            rlRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Y-axis cylinder -> Z axis
            model::drawCylinder(0.2f, 0.4f, 16, YELLOW);
            rlPopMatrix();
        }

        rlPopMatrix();
    }

} // namespace raylibgl::app
