#pragma once

#include "app/DebugControls.hpp"
#include "app/TrackballCamera.hpp"

#include <raylib.h>

namespace raylibgl::app {

    class Application {
    public:
        Application(int width, int height, const char* title);
        ~Application();

        void run();

    private:
        void update();
        void render();
        void drawSceneRlgl();
        void drawCarWithOptionalLighting();
        void drawLightMarkers();

        void loadLightingShader();
        void unloadLightingShader();
        void updateLightingShader();

        int m_width = 0;
        int m_height = 0;
        const char* m_title = nullptr;

        camera::TrackballCamera m_camera{};
        debug::State m_debugState{};

        Shader m_lightingShader{};
        int m_viewPosLoc = -1;
        int m_ambientLoc = -1;
        int m_emissionLoc = -1;
        int m_lightPosLoc[2] = {-1, -1};
        int m_lightColorLoc[2] = {-1, -1};
        int m_lightEnabledLoc[2] = {-1, -1};

        int m_spotPosLoc = -1;
        int m_spotDirLoc = -1;
        int m_spotColorLoc = -1;
        int m_spotEnabledLoc = -1;
    };

} // namespace raylibgl::app
