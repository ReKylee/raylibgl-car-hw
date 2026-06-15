#pragma once

#include "raylib.h"
namespace raylibgl::camera {

    class OrbitCamera {
    public:
        OrbitCamera();

        void Update();

        Camera3D &GetCamera();
        const Camera3D &GetCamera() const;

        void SetTarget(Vector3 target);
        void SetPosition(Vector3 position);

        void SetRotationSpeed(float speed);
        void SetPanSpeed(float speed);
        void SetZoomSpeed(float speed);

    private:
        Camera3D camera_{};

        float rotationSpeed_ = 0.01f;
        float panSpeed_ = 0.0015f;
        float zoomSpeed_ = 0.15f;

        float minDistance_ = 0.25f;
        float maxPitchDot_ = 0.98f;

        float perspectiveFovY_ = 45.0f;
        float orthographicSize_ = 10.0f;

        void Orbit(Vector2 mouseDelta);
        void Pan(Vector2 mouseDelta);
        void Zoom(float wheelMove);
        void ToggleProjection();

        Vector3 GetForward() const;
        Vector3 GetRight() const;
        Vector3 GetUp() const;
        float GetDistanceToTarget() const;
    };
} // namespace raylibgl::camera
