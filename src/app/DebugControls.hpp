#pragma once

#include <raylib.h>

namespace raylibgl::camera {
    class TrackballCamera;
}

namespace raylibgl::app::debug {

    struct State {
        bool panelVisible = true;
        bool showGrid = true;
        bool showAxes = true;
        bool axesFollowModel = false;
        bool wireframe = false;
        bool showLightMarkers = true;
        bool exitRequested = false;
    };

    void Update(State &state, camera::TrackballCamera &camera);
    void DrawOverlay(State &state, camera::TrackballCamera &camera);
    bool WantsMouseCapture(const State &state);

} // namespace raylibgl::app::debug
