#pragma once

#include "floatmath.hpp"

struct CameraInput {
    struct {
        float x;
        float y;
    } movement;
    struct {
        float pitch;
        float yaw;
    } rotation;
    bool lock;
};

struct CameraViewport {
    float x;
    float y;
    float w;
    float h;
};

class Camera {
protected:
    CameraViewport m_viewport = {};

    float m_fieldOfView;
    vector4f m_position;
    vector4f m_rotation;

    matrix4x4f m_translation;
    matrix4x4f m_lookAt;
    matrix4x4f m_projection;
public:
    Camera(CameraViewport viewport, float fieldOfView, vector4f position, vector4f rotation);
    Camera(CameraViewport viewport);
    ~Camera();

    void update(CameraInput& input, float deltaTime, float mouseSensitivity = 1.0f);

    matrix4x4f getViewMatrix();
    matrix4x4f getProjectionMatrix(bool recalculate = false);
    void updateProjectionMatrix() { getProjectionMatrix(true); }

    const CameraViewport& getViewport() const { return m_viewport; }
    void setViewport(CameraViewport viewport) { m_viewport = viewport; }

    const float& getFieldOfView() const { return m_fieldOfView; }
    void setFieldOfView(float fieldOfView) { m_fieldOfView = fieldOfView; }

    const vector4f& getPosition() const { return m_position; }
    void setPosition(vector4f position) { m_position = position; }

    const vector4f& getRotation() const { return m_rotation; }
    void setRotation(vector4f rotation) { m_rotation = rotation; }

    static void CameraUIs();
};