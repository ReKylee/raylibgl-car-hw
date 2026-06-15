#include "app/TrackballCamera.hpp"

#include <cmath>

namespace raylibgl::camera {

    TrackballCamera::TrackballCamera() {
        camera_.position = {10.0f, 10.0f, 10.0f};
        camera_.target = {0.0f, 0.0f, 0.0f};
        camera_.up = {0.0f, 1.0f, 0.0f};
        camera_.fovy = perspectiveFovY_;
        camera_.projection = CAMERA_PERSPECTIVE;
    }

    void TrackballCamera::Update(int screenWidth, int screenHeight) {
        const float wheelMove = GetMouseWheelMove();
        if (wheelMove != 0.0f) {
            Zoom(wheelMove);
        }

        // 'O' toggles orthographic/perspective (design aid). 'P' is reserved for
        // the wireframe toggle required by the assignment.
        if (IsKeyPressed(KEY_O)) {
            ToggleProjection();
        }

        Rotate(screenWidth, screenHeight);
    }

    // --- view / zoom -------------------------------------------------------

    void TrackballCamera::Zoom(float wheelMove) {
        if (camera_.projection == CAMERA_ORTHOGRAPHIC) {
            // Ortho has no perspective distance, so "zoom" scales the view size.
            orthographicSize_ *= std::exp(-wheelMove * zoomSpeed_);
            if (orthographicSize_ < 0.1f) {
                orthographicSize_ = 0.1f;
            }
            camera_.fovy = orthographicSize_;
            return;
        }

        // Perspective: move the eye toward/away from the target (assignment:
        // "Zoom should be achieved by moving the camera closer to the model").
        Vector3 offset = Vector3Subtract(camera_.position, camera_.target);
        float distance = Vector3Length(offset);

        distance *= std::exp(-wheelMove * zoomSpeed_);
        if (distance < minDistance_) {
            distance = minDistance_;
        }

        offset = Vector3Scale(Vector3Normalize(offset), distance);
        camera_.position = Vector3Add(camera_.target, offset);
    }

    void TrackballCamera::ToggleProjection() {
        if (camera_.projection == CAMERA_PERSPECTIVE) {
            camera_.projection = CAMERA_ORTHOGRAPHIC;
            camera_.fovy = orthographicSize_;
        } else {
            camera_.projection = CAMERA_PERSPECTIVE;
            camera_.fovy = perspectiveFovY_;
        }
    }

    // --- trackball (Appendix A) -------------------------------------------

    Vector3 TrackballCamera::ProjectToSphere(float px, float py, int width, int height) {
        // Step 1: canvas pixel -> view-plane coords in [-1, 1], y flipped so up
        // is positive (screen y grows downward).
        const float x = 2.0f * px / static_cast<float>(width) - 1.0f;
        const float y = 1.0f - 2.0f * py / static_cast<float>(height);

        // Step 2: lift onto the sphere (radius^2 = 2 per the appendix). Outside
        // the sphere, clamp z to 0 so the point stays on the silhouette.
        const float d = 2.0f - x * x - y * y;
        const float z = d > 0.0f ? std::sqrt(d) : 0.0f;

        return Vector3Normalize(Vector3{x, y, z});
    }

    void TrackballCamera::Rotate(int screenWidth, int screenHeight) {
        const Vector2 mouse = GetMousePosition();

        // Begin a drag: remember the starting sphere point.
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            dragging_ = true;
            lastVec_ = ProjectToSphere(mouse.x, mouse.y, screenWidth, screenHeight);
            return;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragging_ = false;
        }

        if (!dragging_ || !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            return;
        }

        // Step 3: rotation taking the previous sphere point to the current one.
        const Vector3 cur = ProjectToSphere(mouse.x, mouse.y, screenWidth, screenHeight);
        const Vector3 axis = Vector3CrossProduct(lastVec_, cur);
        const float s = Vector3Length(axis);              // sin(angle) * |a||b|
        const float c = Vector3DotProduct(lastVec_, cur); // cos(angle) * |a||b|

        if (s > 1e-6f) {
            const Vector3 unitAxis = Vector3Scale(axis, 1.0f / s);
            const float angle = std::atan2(s, c);
            const Matrix increment = MatrixRotate(unitAxis, angle);

            // Step 4: accumulate. raylib's MatrixMultiply(A, B) applies A first,
            // so this evaluates to increment * rotation_ = Rn * Rs - the new
            // increment pre-multiplies the stored rotation, exactly as required.
            rotation_ = MatrixMultiply(rotation_, increment);
        }

        lastVec_ = cur;
    }

} // namespace raylibgl::camera
