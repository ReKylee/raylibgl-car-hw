#pragma once

#include <raylib.h>
#include "TrackballCamera.hpp"

namespace raylibgl::app::debug {

    /// @brief Viewer toggles shared between the keyboard shortcuts and the debug panel.
    struct State {
        bool panelVisible = true;       ///< Debug panel expanded (vs. collapsed to a button).
        bool showGrid = true;           ///< Draw the floor grid.
        bool showAxes = true;           ///< Draw the XYZ axes.
        bool axesFollowModel = false;   ///< Rotate the axes with the model (vs. fixed world axes).
        bool wireframe = false;         ///< Render the model as wireframe.
        bool showLightMarkers = true;   ///< Draw marker spheres at the light positions.
        bool exitRequested = false;     ///< Set by the panel's exit button to end the run loop.
    };

    /// @brief Apply keyboard shortcuts to @p state and @p camera for the current frame.
    void Update(State &state, camera::TrackballCamera &camera);
    /// @brief Draw the FPS readout and debug control panel, mutating @p state / @p camera on input.
    void DrawOverlay(State &state, camera::TrackballCamera &camera);
    /// @brief Whether the cursor is over UI this frame, so the model shouldn't capture the drag.
    bool WantsMouseCapture(const State &state);

} // namespace raylibgl::app::debug
