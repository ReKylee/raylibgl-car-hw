#pragma once

#include "app/CameraControls.hpp"
#include "app/DebugControls.hpp"


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

        camera::OrbitCamera m_camera{};
    };

} // namespace raylibgl::app
