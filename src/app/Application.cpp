#include "app/Application.hpp"

#include "model/Primitives.hpp"
#include "model/Scooby-van.hpp"

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

        if (IsKeyPressed(KEY_P)) {
            m_wireframe = !m_wireframe;
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

        // The Mystery Machine van. Each part picks the filled or wireframe
        // raylib call based on `wire` (the 'P' toggle).
        model::drawCar(m_wireframe);

        rlPopMatrix();
    }

} // namespace raylibgl::app
