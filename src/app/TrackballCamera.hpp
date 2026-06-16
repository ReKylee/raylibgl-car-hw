#pragma once

#include <raylib.h>

namespace raylibgl::camera {

    class TrackballCamera {
    public:
        TrackballCamera();

        void Update(int screenWidth, int screenHeight, bool allowMouseInput = true);

        [[nodiscard]] Camera3D GetCamera() const { return camera_; }
        [[nodiscard]] const Matrix& GetRotation() const { return rotation_; }
        [[nodiscard]] bool IsPerspective() const { return camera_.projection == CAMERA_PERSPECTIVE; }

        void ResetRotation();
        void ToggleProjection();

    private:
        void Zoom(float wheelMove);
        void Rotate(int screenWidth, int screenHeight);

        [[nodiscard]] static Vector3 ProjectToSphereView(float px, float py, int width, int height);
        [[nodiscard]] static Vector3 AnyOrthogonalAxis(Vector3 v);
        [[nodiscard]] Matrix ViewToWorldBasis() const;
        [[nodiscard]] Matrix ViewRotationToModelRotation(Matrix viewRotation) const;

        Camera3D camera_{};
        Matrix rotation_{};

        Vector3 dragStartVec_{0.0f, 0.0f, 1.0f};
        Matrix dragStartRotation_{};
        bool dragging_ = false;

        float perspectiveFovY_ = 45.0f;
        float orthographicSize_ = 7.5f;
        float zoomSpeed_ = 0.12f;
        float minDistance_ = 2.0f;
        float maxDistance_ = 30.0f;
        float trackballSensitivity_ = 1.0f;
    };

} // namespace raylibgl::camera
