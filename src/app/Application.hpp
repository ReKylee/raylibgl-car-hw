#pragma once

#include "app/DebugControls.hpp"
#include "app/TrackballCamera.hpp"


namespace raylibgl::app {

    class Application {
    public:
        Application(int width, int height, const char *title);
        ~Application();

        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        void run();

    private:
        void createTestScene();
        void update();
        void render();

        void drawSceneRlgl();

    private:
        int m_width = 0;
        int m_height = 0;
        const char *m_title;

        camera::TrackballCamera m_camera{};

        bool m_showAxes = true; // toggled with 'A'
    };

} // namespace raylibgl::app
