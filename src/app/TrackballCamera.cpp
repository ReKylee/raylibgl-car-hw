#include "app/TrackballCamera.hpp"

#include <raymath.h>

#include <algorithm>
#include <cmath>

namespace raylibgl::camera {
    namespace {

        constexpr float ROTATION_EPSILON = 1e-6f;
        constexpr float OPPOSITE_VECTOR_DOT = -0.999999f;

        constexpr float HORIZONTAL_SIGN = 1.0f;
        constexpr float VERTICAL_SIGN = -1.0f;

    } // namespace

    TrackballCamera::TrackballCamera() {
        camera_.position = {10.0f, 10.0f, 10.0f};
        camera_.target = {0.0f, 0.0f, 0.0f};
        camera_.up = {0.0f, 1.0f, 0.0f};
        camera_.fovy = perspectiveFovY_;
        camera_.projection = CAMERA_PERSPECTIVE;

        rotation_ = MatrixIdentity();
        dragStartRotation_ = MatrixIdentity();
    }

    void TrackballCamera::Update(int screenWidth, int screenHeight, bool allowMouseInput) {
        if (!allowMouseInput) {
            dragging_ = false;
            return;
        }

        const float wheelMove = GetMouseWheelMove();
        if (wheelMove != 0.0f) {
            Zoom(wheelMove);
        }

        Rotate(screenWidth, screenHeight);
    }

    void TrackballCamera::ResetRotation() {
        rotation_ = MatrixIdentity();
        dragStartRotation_ = MatrixIdentity();
        dragStartVec_ = {0.0f, 0.0f, 1.0f};
        dragging_ = false;
    }

    void TrackballCamera::ToggleProjection() {
        if (camera_.projection == CAMERA_PERSPECTIVE) {
            camera_.projection = CAMERA_ORTHOGRAPHIC;
            camera_.fovy = orthographicSize_;
            return;
        }

        camera_.projection = CAMERA_PERSPECTIVE;
        camera_.fovy = perspectiveFovY_;
    }

    void TrackballCamera::Zoom(float wheelMove) {
        if (camera_.projection == CAMERA_ORTHOGRAPHIC) {
            orthographicSize_ *= std::exp(-wheelMove * zoomSpeed_);
            orthographicSize_ = std::clamp(orthographicSize_, 0.5f, 40.0f);
            camera_.fovy = orthographicSize_;
            return;
        }

        Vector3 offset = Vector3Subtract(camera_.position, camera_.target);
        float distance = Vector3Length(offset);

        distance *= std::exp(-wheelMove * zoomSpeed_);
        distance = std::clamp(distance, minDistance_, maxDistance_);

        offset = Vector3Scale(Vector3Normalize(offset), distance);
        camera_.position = Vector3Add(camera_.target, offset);
    }

    Vector3 TrackballCamera::ProjectToSphereView(float px, float py, int width, int height) {
        const int safeWidth = std::max(width, 1);
        const int safeHeight = std::max(height, 1);

        // Use the smaller dimension so the virtual trackball stays circular in
        // wide/tall windows. Coordinates are centered around the viewport center.
        const float diameter = static_cast<float>(std::max(1, std::min(safeWidth, safeHeight)));
        const float invDiameter = 1.0f / diameter;

        // View-space convention used here:
        // +X is screen-right before the sign correction below.
        // +Y is screen-up.
        // +Z points toward the viewer/camera.
        const float rawX = (2.0f * px - static_cast<float>(safeWidth)) * invDiameter;
        const float rawY = (static_cast<float>(safeHeight) - 2.0f * py) * invDiameter;

        const float x = HORIZONTAL_SIGN * rawX;
        const float y = VERTICAL_SIGN * rawY;
        const float d = x * x + y * y;

        // Classic arcball projection: use the sphere inside the radius, and clamp
        // to the sphere rim outside. The returned vector is always unit length.
        if (d <= 1.0f) {
            return Vector3{x, y, std::sqrt(1.0f - d)};
        }

        const float invLength = 1.0f / std::sqrt(d);
        return Vector3{x * invLength, y * invLength, 0.0f};
    }

    Vector3 TrackballCamera::AnyOrthogonalAxis(Vector3 v) {
        const Vector3 xAxis{1.0f, 0.0f, 0.0f};
        const Vector3 yAxis{0.0f, 1.0f, 0.0f};

        Vector3 axis = Vector3CrossProduct(v, xAxis);
        if (Vector3Length(axis) < ROTATION_EPSILON) {
            axis = Vector3CrossProduct(v, yAxis);
        }

        return Vector3Normalize(axis);
    }

    Matrix TrackballCamera::ViewToWorldBasis() const {
        const Vector3 cameraBackward = Vector3Normalize(Vector3Subtract(camera_.position, camera_.target));
        const Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(camera_.up, cameraBackward));
        const Vector3 cameraUp = Vector3Normalize(Vector3CrossProduct(cameraBackward, cameraRight));

        // This matrix maps a view-space direction to a world-space direction:
        // world = right*x + up*y + backward*z.
        // There is no translation because this is a direction/basis transform.
        return Matrix{
            cameraRight.x,
            cameraUp.x,
            cameraBackward.x,
            0.0f,
            cameraRight.y,
            cameraUp.y,
            cameraBackward.y,
            0.0f,
            cameraRight.z,
            cameraUp.z,
            cameraBackward.z,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
        };
    }

    Matrix TrackballCamera::ViewRotationToModelRotation(Matrix viewRotation) const {
        const Matrix viewToWorld = ViewToWorldBasis();
        const Matrix worldToView = MatrixInvert(viewToWorld);

        // We want the same result as multiplying the OpenGL model-view matrix:
        //     MV_new = R_view * MV_start
        // but this class stores only the model matrix M, while MV = V * M.
        // Therefore:
        //     V * M_new = R_view * V * M_start
        //     M_new = V^-1 * R_view * V * M_start
        // With viewToWorld = V^-1 and worldToView = V, this function returns:
        //     V^-1 * R_view * V
        return MatrixMultiply(MatrixMultiply(viewToWorld, viewRotation), worldToView);
    }

    void TrackballCamera::Rotate(int screenWidth, int screenHeight) {
        const Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            dragging_ = true;
            dragStartVec_ = ProjectToSphereView(mouse.x, mouse.y, screenWidth, screenHeight);
            dragStartRotation_ = rotation_;
            return;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragging_ = false;
            return;
        }

        if (!dragging_ || !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            return;
        }

        const Vector3 currentVec = ProjectToSphereView(mouse.x, mouse.y, screenWidth, screenHeight);
        const float dot = std::clamp(Vector3DotProduct(dragStartVec_, currentVec), -1.0f, 1.0f);

        Vector3 axisView = Vector3CrossProduct(dragStartVec_, currentVec);
        float sinAngle = Vector3Length(axisView);

        float angle = 0.0f;
        if (sinAngle > ROTATION_EPSILON) {
            axisView = Vector3Scale(axisView, 1.0f / sinAngle);
            angle = std::atan2(sinAngle, dot);
        } else if (dot < OPPOSITE_VECTOR_DOT) {
            axisView = AnyOrthogonalAxis(dragStartVec_);
            angle = PI;
        } else {
            rotation_ = dragStartRotation_;
            return;
        }

        const Matrix viewIncrement = MatrixRotate(axisView, angle * trackballSensitivity_);
        const Matrix modelIncrement = ViewRotationToModelRotation(viewIncrement);

        // Equivalent to applying the trackball rotation to the model-view matrix:
        //     MV_new = R_view * MV_start
        // but stored as model rotation only.
        rotation_ = MatrixMultiply(modelIncrement, dragStartRotation_);
    }

} // namespace raylibgl::camera
