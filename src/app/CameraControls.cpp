#include "CameraControls.hpp"

#include <raymath.h>

namespace raylibgl::camera {

    OrbitCamera::OrbitCamera() {
        camera_.position = {10.0f, 10.0f, 10.0f};
        camera_.target = {0.0f, 0.0f, 0.0f};
        camera_.up = {0.0f, 1.0f, 0.0f};
        camera_.fovy = perspectiveFovY_;
        camera_.projection = CAMERA_PERSPECTIVE;
    }

    void OrbitCamera::Update() {
        const Vector2 mouseDelta = GetMouseDelta();
        const float wheelMove = GetMouseWheelMove();

        if (wheelMove != 0.0f) {
            Zoom(wheelMove);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            const bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

            if (shiftDown) {
                Pan(mouseDelta);
            } else {
                Orbit(mouseDelta);
            }
        }

        if (IsKeyPressed(KEY_P)) {
            ToggleProjection();
        }
    }

    Camera3D &OrbitCamera::GetCamera() { return camera_; }

    const Camera3D &OrbitCamera::GetCamera() const { return camera_; }

    void OrbitCamera::SetTarget(Vector3 target) { camera_.target = target; }

    void OrbitCamera::SetPosition(Vector3 position) { camera_.position = position; }

    void OrbitCamera::SetRotationSpeed(float speed) { rotationSpeed_ = speed; }

    void OrbitCamera::SetPanSpeed(float speed) { panSpeed_ = speed; }

    void OrbitCamera::SetZoomSpeed(float speed) { zoomSpeed_ = speed; }

    void OrbitCamera::Orbit(Vector2 mouseDelta) {
        Vector3 offset = Vector3Subtract(camera_.position, camera_.target);

        const Vector3 worldUp = {0.0f, 1.0f, 0.0f};

        const float yaw = -mouseDelta.x * rotationSpeed_;
        offset = Vector3RotateByAxisAngle(offset, worldUp, yaw);

        Vector3 forward = Vector3Normalize(Vector3Negate(offset));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, worldUp));

        const float pitch = -mouseDelta.y * rotationSpeed_;
        Vector3 pitchedOffset = Vector3RotateByAxisAngle(offset, right, pitch);

        Vector3 pitchedDirection = Vector3Normalize(pitchedOffset);

        if (std::fabs(Vector3DotProduct(pitchedDirection, worldUp)) < maxPitchDot_) {
            offset = pitchedOffset;
        }

        camera_.position = Vector3Add(camera_.target, offset);
        camera_.up = worldUp;
    }

    void OrbitCamera::Pan(Vector2 mouseDelta) {
        const float distance = GetDistanceToTarget();
        const float scaledPanSpeed = panSpeed_ * distance;

        Vector3 right = GetRight();
        Vector3 up = GetUp();

        Vector3 pan = Vector3Add(Vector3Scale(right, -mouseDelta.x * scaledPanSpeed),
                                 Vector3Scale(up, mouseDelta.y * scaledPanSpeed));

        camera_.position = Vector3Add(camera_.position, pan);
        camera_.target = Vector3Add(camera_.target, pan);
    }

    void OrbitCamera::Zoom(float wheelMove) {
        if (camera_.projection == CAMERA_ORTHOGRAPHIC) {
            orthographicSize_ *= std::exp(-wheelMove * zoomSpeed_);

            if (orthographicSize_ < 0.1f) {
                orthographicSize_ = 0.1f;
            }

            camera_.fovy = orthographicSize_;
            return;
        }

        Vector3 offset = Vector3Subtract(camera_.position, camera_.target);
        float distance = Vector3Length(offset);

        distance *= std::exp(-wheelMove * zoomSpeed_);

        if (distance < minDistance_) {
            distance = minDistance_;
        }

        offset = Vector3Scale(Vector3Normalize(offset), distance);
        camera_.position = Vector3Add(camera_.target, offset);
    }

    void OrbitCamera::ToggleProjection() {
        if (camera_.projection == CAMERA_PERSPECTIVE) {
            camera_.projection = CAMERA_ORTHOGRAPHIC;
            camera_.fovy = orthographicSize_;
        } else {
            camera_.projection = CAMERA_PERSPECTIVE;
            camera_.fovy = perspectiveFovY_;
        }
    }

    Vector3 OrbitCamera::GetForward() const {
        return Vector3Normalize(Vector3Subtract(camera_.target, camera_.position));
    }

    Vector3 OrbitCamera::GetRight() const { return Vector3Normalize(Vector3CrossProduct(GetForward(), camera_.up)); }

    Vector3 OrbitCamera::GetUp() const { return Vector3Normalize(Vector3CrossProduct(GetRight(), GetForward())); }

    float OrbitCamera::GetDistanceToTarget() const { return Vector3Distance(camera_.position, camera_.target); }
} // namespace raylibgl::camera
