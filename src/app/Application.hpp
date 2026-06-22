#pragma once

#include "app/DebugControls.hpp"
#include "app/TrackballCamera.hpp"

#include <raylib.h>

namespace raylibgl::app {

    class Application {
    public:
        /// @brief Create the window and load the car resources and lighting shader.
        Application(int width, int height, const char* title);
        /// @brief Release the shader, car resources and window.
        ~Application();

        /// @brief Run the update/render loop until the window or the debug panel requests exit.
        void run();

    private:
        /// @brief Advance per-frame state: viewport size, debug input and camera.
        void update();
        /// @brief Render one frame: 3D scene, then the debug overlay.
        void render();
        /// @brief Draw the scene in rlgl immediate mode, applying the trackball rotation to the model.
        void drawSceneRlgl();
        /// @brief Draw the car: wireframe as-is, otherwise in the lit/emissive/transparent passes.
        void drawCarWithOptionalLighting();
        /// @brief Draw marker spheres at the two positional light positions.
        void drawLightMarkers();

        /// @brief Compile the lighting shader and cache its uniform locations and constants.
        void loadLightingShader();
        /// @brief Release the lighting shader if loaded.
        void unloadLightingShader();
        /// @brief Push the per-frame uniforms (view position, lights, van-attached spotlights).
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
