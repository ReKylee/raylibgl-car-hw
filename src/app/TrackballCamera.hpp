#pragma once

#include <raylib.h>
#include <raymath.h>

namespace raylibgl::camera {

    // Combined view + trackball for the model viewer.
    //
    //  - The Camera3D provides the VIEW: a fixed eye looking at the origin, with
    //    mouse-wheel zoom and a perspective/orthographic toggle.
    //  - The trackball provides the MODEL ROTATION: left-drag accumulates a
    //    rotation matrix (assignment Appendix A) that the caller applies to the
    //    geometry via the ModelView (rlMultMatrixf). The camera itself never
    //    rotates - the world spins about the origin in front of a fixed eye.
    class TrackballCamera {
    public:
        TrackballCamera();

        // Drive both pieces once per frame with the current viewport size.
        void Update(int screenWidth, int screenHeight);

        Camera3D &GetCamera() { return camera_; }
        const Camera3D &GetCamera() const { return camera_; }

        // Accumulated model rotation, to push onto the ModelView matrix.
        Matrix GetRotation() const { return rotation_; }

    private:
        // --- view / zoom ---
        void Zoom(float wheelMove);
        void ToggleProjection();

        // --- trackball ---
        // Canvas pixel -> point on the virtual sphere (Appendix A, steps 1-2).
        static Vector3 ProjectToSphere(float px, float py, int width, int height);
        void Rotate(int screenWidth, int screenHeight);

        Camera3D camera_{};

        // zoom / projection tuning (carried over from the orbit camera)
        float zoomSpeed_ = 0.15f;
        float minDistance_ = 0.25f;
        float perspectiveFovY_ = 45.0f;
        float orthographicSize_ = 10.0f;

        // trackball state
        Matrix rotation_ = MatrixIdentity(); // cumulative rotation (Rs)
        Vector3 lastVec_ = {0.0f, 0.0f, 1.0f}; // previous sphere point
        bool dragging_ = false;
    };

} // namespace raylibgl::camera
