#include "app/Application.hpp"

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

        m_camera.Update();
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

        rlPushMatrix();

        rlBegin(RL_TRIANGLES);

        rlColor3f(1.0f, 0.0f, 0.0f);
        rlVertex3f(0.0f, 1.0f, 0.0f);

        rlColor3f(0.0f, 1.0f, 0.0f);
        rlVertex3f(-1.0f, 0.0f, 0.0f);

        rlColor3f(0.0f, 0.0f, 1.0f);
        rlVertex3f(1.0f, 0.0f, 0.0f);

        rlEnd();

        rlPopMatrix();
    }

} // namespace raylibgl::app
