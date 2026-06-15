#include "app/Application.hpp"

namespace {

    constexpr int DEFAULT_WINDOW_WIDTH = 1280;
    constexpr int DEFAULT_WINDOW_HEIGHT = 720;
    constexpr const char *WINDOW_TITLE = "raylibgl car";
} // namespace

int main() {
    raylibgl::app::Application app(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, WINDOW_TITLE);
    app.run();

    return 0;
}
