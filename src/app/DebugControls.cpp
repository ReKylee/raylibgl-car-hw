#include "app/DebugControls.hpp"

#include "app/TrackballCamera.hpp"

#include "raygui.h"

#include <algorithm>

namespace raylibgl::app::debug {
    namespace {

        constexpr int PANEL_WIDTH = 340;
        constexpr int PANEL_HEIGHT = 384;
        constexpr int PANEL_MARGIN = 14;
        constexpr int PANEL_TOP = 44; // leaves the FPS pill clear at the top-left

        // GuiWindowBox draws a fixed-height status bar for its title (raygui default).
        constexpr float STATUSBAR_HEIGHT = 24.0f;

        // FPS pill + collapsed "open" button live together along the top edge.
        constexpr Rectangle FPS_PILL{12.0f, 10.0f, 92.0f, 26.0f};
        constexpr Rectangle OPEN_BUTTON{112.0f, 10.0f, 132.0f, 26.0f};

        constexpr float REFERENCE_HEIGHT = 900.0f;
        constexpr float MIN_SCALE = 0.85f;
        constexpr float MAX_SCALE = 1.20f;

        bool styleLoaded = false;

        float UiScale() {
            const float raw = static_cast<float>(GetScreenHeight()) / REFERENCE_HEIGHT;
            return std::clamp(raw, MIN_SCALE, MAX_SCALE);
        }

        float ScaleF(float value, float uiScale) {
            return value * uiScale;
        }

        int ScaleI(int value, float uiScale) {
            return static_cast<int>(static_cast<float>(value) * uiScale + 0.5f);
        }

        Rectangle PanelBounds() {
            const float scale = UiScale();
            const int margin = ScaleI(PANEL_MARGIN, scale);
            const int width = std::min(ScaleI(PANEL_WIDTH, scale), GetScreenWidth() - margin * 2);
            const int height = ScaleI(PANEL_HEIGHT, scale);

            return Rectangle{
                static_cast<float>(margin),
                static_cast<float>(PANEL_TOP),
                static_cast<float>(width),
                static_cast<float>(height),
            };
        }

        // Load the amber style once. GuiLoadStyle replaces the whole global style
        // (colors, metrics and font), so after this call we never touch GuiSetStyle --
        // every widget below paints itself straight from the loaded .rgs. raygui keeps
        // it in global state, so reloading per-frame would just re-read the file for nothing.
        void LoadStyleOnce() {
            if (styleLoaded) {
                return;
            }
            styleLoaded = true;

            // Resolve against the executable directory so it works no matter the CWD.
            const char* path =
                TextFormat("%sassets/styles/amber/style_amber.rgs", GetApplicationDirectory());
            GuiLoadStyle(path);
        }

        // Small framed FPS readout. The frame colors come from the active style so it
        // matches whatever .rgs is loaded; DrawFPS itself is raylib's built-in counter.
        void DrawFpsPill() {
            const Color bg = GetColor(static_cast<unsigned int>(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            const Color border = GetColor(static_cast<unsigned int>(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

            DrawRectangleRounded(FPS_PILL, 0.25f, 8, ColorAlpha(bg, 0.92f));
            DrawRectangleRoundedLines(FPS_PILL, 0.25f, 8, border);
            DrawFPS(static_cast<int>(FPS_PILL.x) + 8, static_cast<int>(FPS_PILL.y) + 5);
        }

    } // namespace

    void Update(State& state, camera::TrackballCamera& camera) {
        if (IsKeyPressed(KEY_P)) {
            state.wireframe = !state.wireframe;
        }

        if (IsKeyPressed(KEY_A)) {
            state.showAxes = !state.showAxes;
        }

        if (IsKeyPressed(KEY_L)) {
            state.showLightMarkers = !state.showLightMarkers;
        }

        if (IsKeyPressed(KEY_G)) {
            state.showGrid = !state.showGrid;
        }

        if (IsKeyPressed(KEY_X)) {
            state.axesFollowModel = !state.axesFollowModel;
        }

        if (IsKeyPressed(KEY_O)) {
            camera.ToggleProjection();
        }

        if (IsKeyPressed(KEY_R)) {
            camera.ResetRotation();
        }
    }

    void DrawOverlay(State& state, camera::TrackballCamera& camera) {
        LoadStyleOnce();
        DrawFpsPill();

        // Collapsed: the panel lives behind a single button next to the FPS pill.
        if (!state.panelVisible) {
            if (GuiButton(OPEN_BUTTON, "#140# Debug panel")) {
                state.panelVisible = true;
            }
            return;
        }

        const float scale = UiScale();
        const Rectangle panel = PanelBounds();

        // Titled, closable container -- raygui paints the background, border, title bar
        // and the close button entirely from the style.
        if (GuiWindowBox(panel, "#140# VIEWER DEBUG")) {
            state.panelVisible = false;
        }

        // Layout cursor. raygui has no layout manager, so we step a uniform row height
        // down a single column (the idiomatic raygui example pattern).
        const float pad = ScaleF(12.0f, scale);
        const float rowH = ScaleF(24.0f, scale);
        const float gap = ScaleF(6.0f, scale);
        const float lineH = ScaleF(16.0f, scale);
        const float checkSize = ScaleF(18.0f, scale);
        const float btnH = ScaleF(26.0f, scale);

        const float x = panel.x + pad;
        const float w = panel.width - pad * 2.0f;
        float y = panel.y + STATUSBAR_HEIGHT + pad;

        auto section = [&](const char* label) {
            GuiLine(Rectangle{x, y, w, lineH}, label);
            y += lineH + gap;
        };
        auto check = [&](const char* label, bool* value) {
            const bool changed = GuiCheckBox(Rectangle{x, y, checkSize, checkSize}, label, value);
            y += rowH;
            return changed;
        };
        auto button = [&](const char* label) {
            const bool clicked = GuiButton(Rectangle{x, y, w, btnH}, label);
            y += btnH + gap;
            return clicked;
        };
        auto hint = [&](const char* label) {
            GuiLabel(Rectangle{x, y, w, rowH}, label);
            y += rowH;
        };

        // ---- Scene toggles ------------------------------------------------
        section("Scene");
        check("Wireframe polygons  [P]", &state.wireframe);
        check("XYZ axes  [A]", &state.showAxes);

        // "Axes rotate with model" only means something when the axes are drawn,
        // so grey it out (style-driven disabled look) when axes are hidden.
        if (!state.showAxes) {
            GuiSetState(STATE_DISABLED);
        }
        check("Axes rotate with model  [X]", &state.axesFollowModel);
        GuiSetState(STATE_NORMAL);

        check("Floor grid  [G]", &state.showGrid);
        check("Light source markers  [L]", &state.showLightMarkers);

        // ---- Camera -------------------------------------------------------
        y += gap;
        section("Camera");

        // Projection is a binary state -> a checkbox that mirrors the camera.
        bool perspective = camera.IsPerspective();
        if (check("Perspective projection  [O]", &perspective)) {
            camera.ToggleProjection();
        }

        // Resetting the rotation is a one-shot action -> a button.
        if (button("Reset rotation  [R]")) {
            camera.ResetRotation();
        }

        // ---- View controls ------------------------------------------------
        y += gap;
        section("View controls");
        hint("Drag  -  trackball rotation");
        hint("Wheel  -  zoom in / out");

        if (button("Exit viewer  [F10]")) {
            state.exitRequested = true;
        }
    }

    bool WantsMouseCapture(const State& state) {
        // Swallow clicks on the collapsed "open" button so they don't rotate the model.
        if (!state.panelVisible) {
            return CheckCollisionPointRec(GetMousePosition(), OPEN_BUTTON);
        }

        return CheckCollisionPointRec(GetMousePosition(), PanelBounds());
    }

} // namespace raylibgl::app::debug
