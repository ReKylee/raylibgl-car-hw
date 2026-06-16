#include "app/DebugControls.hpp"

#include "app/TrackballCamera.hpp"

#include "raygui.h"

#include <algorithm>

namespace raylibgl::app::debug {
    namespace {

        constexpr int PANEL_WIDTH = 360;
        constexpr int PANEL_HEIGHT = 430;
        constexpr int PANEL_MARGIN = 14;
        constexpr int ROW_HEIGHT = 28;
        constexpr int HEADER_HEIGHT = 34;
        constexpr int CHECK_SIZE = 18;
        constexpr int BUTTON_HEIGHT = 26;

        constexpr float REFERENCE_HEIGHT = 900.0f;
        constexpr float MIN_SCALE = 0.85f;
        constexpr float MAX_SCALE = 1.20f;

        constexpr Color PANEL_BG{24, 25, 28, 230};
        constexpr Color PANEL_HEADER{38, 39, 43, 245};
        constexpr Color PANEL_BORDER{120, 125, 135, 150};
        constexpr Color ACCENT{255, 150, 70, 255};
        constexpr Color TEXT_MAIN{232, 232, 232, 255};
        constexpr Color TEXT_MUTED{165, 170, 178, 255};
        constexpr Color FPS_BG{18, 18, 20, 210};

        bool styleReady = false;

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
                static_cast<float>(margin),
                static_cast<float>(width),
                static_cast<float>(height),
            };
        }

        void ApplyGuiStyle() {
            if (styleReady) {
                return;
            }

            styleReady = true;

            GuiSetStyle(DEFAULT, TEXT_SIZE, 17);
            GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
            GuiSetStyle(DEFAULT, TEXT_PADDING, 8);
            GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(PANEL_BG));

            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(TEXT_MAIN));
            GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(WHITE));
            GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, ColorToInt(WHITE));
            GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, ColorToInt(TEXT_MUTED));

            GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(Color{48, 50, 56, 255}));
            GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, ColorToInt(Color{68, 72, 82, 255}));
            GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, ColorToInt(ACCENT));

            GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(Color{95, 100, 112, 255}));
            GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(ACCENT));
            GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(Color{255, 190, 120, 255}));
        }

        void DrawFpsPill() {
            constexpr float x = 12.0f;
            constexpr float y = 10.0f;
            constexpr float width = 92.0f;
            constexpr float height = 26.0f;

            const Rectangle bounds{x, y, width, height};

            DrawRectangleRounded(bounds, 0.25f, 8, FPS_BG);
            DrawRectangleRoundedLines(bounds, 0.25f, 8, PANEL_BORDER);
            DrawFPS(static_cast<int>(x) + 8, static_cast<int>(y) + 5);
        }

        void DrawShortcutLine(int x, int y, const char* key, const char* description) {
            DrawText(key, x, y, 16, ACCENT);
            DrawText(description, x + 56, y, 16, TEXT_MUTED);
        }

    } // namespace

    void Update(State& state, camera::TrackballCamera& camera) {
        if (IsKeyPressed(KEY_F1)) {
            state.panelVisible = !state.panelVisible;
        }

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
        ApplyGuiStyle();
        DrawFpsPill();

        if (!state.panelVisible) {
            DrawText("F1 debug", 116, 15, 16, TEXT_MUTED);
            return;
        }

        const float scale = UiScale();
        const Rectangle panel = PanelBounds();

        const int padding = ScaleI(14, scale);
        const int rowHeight = ScaleI(ROW_HEIGHT, scale);
        const int checkSize = ScaleI(CHECK_SIZE, scale);

        DrawRectangleRounded(panel, 0.04f, 8, PANEL_BG);
        DrawRectangleRoundedLines(panel, 0.04f, 8, PANEL_BORDER);

        DrawRectangleRec(
            Rectangle{
                panel.x,
                panel.y,
                panel.width,
                ScaleF(static_cast<float>(HEADER_HEIGHT), scale),
            },
            PANEL_HEADER);

        DrawRectangleRec(
            Rectangle{
                panel.x,
                panel.y,
                panel.width,
                2.0f,
            },
            ACCENT);

        const int x = static_cast<int>(panel.x) + padding;
        int y = static_cast<int>(panel.y) + ScaleI(9, scale);

        DrawText("VIEWER DEBUG", x, y, ScaleI(18, scale), TEXT_MAIN);

        const Rectangle closeButton{
            panel.x + panel.width - ScaleF(30.0f, scale),
            panel.y + ScaleF(6.0f, scale),
            ScaleF(22.0f, scale),
            ScaleF(22.0f, scale),
        };

        if (GuiButton(closeButton, "x")) {
            state.panelVisible = false;
        }

        y = static_cast<int>(panel.y) + ScaleI(HEADER_HEIGHT + 14, scale);

        DrawText("Scene", x, y, ScaleI(16, scale), TEXT_MAIN);
        y += ScaleI(24, scale);

        GuiCheckBox(
            Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(checkSize),
                static_cast<float>(checkSize),
            },
            "Wireframe polygons  [P]", &state.wireframe);

        y += rowHeight;

        GuiCheckBox(
            Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(checkSize),
                static_cast<float>(checkSize),
            },
            "XYZ axes  [A]", &state.showAxes);

        y += rowHeight;

        GuiCheckBox(
            Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(checkSize),
                static_cast<float>(checkSize),
            },
            "Axes rotate with model  [X]", &state.axesFollowModel);

        y += rowHeight;

        GuiCheckBox(
            Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(checkSize),
                static_cast<float>(checkSize),
            },
            "Floor grid  [G]", &state.showGrid);

        y += rowHeight;

        GuiCheckBox(
            Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(checkSize),
                static_cast<float>(checkSize),
            },
            "Light source markers  [L]", &state.showLightMarkers);

        y += ScaleI(42, scale);

        DrawText("Camera", x, y, ScaleI(16, scale), TEXT_MAIN);
        y += ScaleI(24, scale);

        const char* projection = camera.IsPerspective() ? "Projection: Perspective" : "Projection: Orthographic";
        DrawText(projection, x, y, ScaleI(16, scale), TEXT_MUTED);
        y += ScaleI(24, scale);

        if (GuiButton(
                Rectangle{
                    static_cast<float>(x),
                    static_cast<float>(y),
                    ScaleF(142.0f, scale),
                    ScaleF(static_cast<float>(BUTTON_HEIGHT), scale),
                },
                "Toggle [O]")) {
            camera.ToggleProjection();
        }

        if (GuiButton(
                Rectangle{
                    static_cast<float>(x) + ScaleF(154.0f, scale),
                    static_cast<float>(y),
                    ScaleF(142.0f, scale),
                    ScaleF(static_cast<float>(BUTTON_HEIGHT), scale),
                },
                "Reset [R]")) {
            camera.ResetRotation();
        }

        y += ScaleI(52, scale);

        DrawText("Controls", x, y, ScaleI(16, scale), TEXT_MAIN);
        y += ScaleI(24, scale);

        DrawShortcutLine(x, y, "Drag", "virtual trackball rotation");
        y += ScaleI(22, scale);

        DrawShortcutLine(x, y, "Wheel", "zoom by camera distance");
        y += ScaleI(22, scale);

        DrawShortcutLine(x, y, "F1", "show/hide this panel");
        y += ScaleI(22, scale);

        DrawShortcutLine(x, y, "F10", "exit");
    }

    bool WantsMouseCapture(const State& state) {
        if (!state.panelVisible) {
            return false;
        }

        return CheckCollisionPointRec(GetMousePosition(), PanelBounds());
    }

} // namespace raylibgl::app::debug
