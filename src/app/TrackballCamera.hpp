#pragma once

#include <raylib.h>

namespace raylibgl::camera {

    /**
     * @brief Virtual-trackball camera: a fixed eye that views a model rotated about the
     *        origin via an accumulated rotation matrix.
     *
     * Left-drag spins the model (arcball), the mouse wheel zooms, and the projection can
     * toggle between perspective and orthographic. The rotation is exposed as a matrix so
     * the scene can apply it to the model rather than moving the eye.
     */
    class TrackballCamera {
    public:
        TrackballCamera();

        /**
         * @brief Process one frame of input: wheel zoom and left-drag rotation.
         * @param screenWidth    Current viewport width, in pixels.
         * @param screenHeight   Current viewport height, in pixels.
         * @param allowMouseInput When false, input is ignored and any drag is cancelled
         *                        (e.g. while the cursor is over UI).
         */
        void Update(int screenWidth, int screenHeight, bool allowMouseInput = true);

        /// @brief The raylib camera (eye, target, up, projection) for BeginMode3D.
        [[nodiscard]] Camera3D GetCamera() const { return camera_; }
        /// @brief The accumulated model rotation, to be applied around the origin.
        [[nodiscard]] const Matrix& GetRotation() const { return rotation_; }
        /// @brief Whether the camera is currently in perspective (vs. orthographic) projection.
        [[nodiscard]] bool IsPerspective() const { return camera_.projection == CAMERA_PERSPECTIVE; }

        /// @brief Reset the model rotation back to identity.
        void ResetRotation();
        /// @brief Switch between perspective and orthographic projection.
        void ToggleProjection();

    private:
        /// @brief Apply a wheel-driven zoom (dolly in perspective, scale in orthographic).
        void Zoom(float wheelMove);
        /// @brief Update the accumulated rotation from the current left-drag, if any.
        void Rotate(int screenWidth, int screenHeight);

        /**
         * @brief Project a screen-space pixel onto the unit arcball sphere in view space.
         * @return A unit vector: on the sphere inside the radius, clamped to the rim outside.
         */
        [[nodiscard]] static Vector3 ProjectToSphereView(float px, float py, int width, int height);
        /// @brief Any unit axis orthogonal to @p v (used for the 180-degree flip case).
        [[nodiscard]] static Vector3 AnyOrthogonalAxis(Vector3 v);
        /// @brief Basis mapping a view-space direction to world space (no translation).
        [[nodiscard]] Matrix ViewToWorldBasis() const;
        /// @brief Convert a view-space rotation into the equivalent model-space rotation.
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
