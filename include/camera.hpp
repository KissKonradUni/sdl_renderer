#pragma once

#include "floatmath.hpp"
#include "shader.hpp"

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

struct CameraUniformBufferData : UniformBufferData {
    matrix4x4f view;
    matrix4x4f projection;
    vector4f m_position;
    vector4f m_forward;
};

class Camera {
protected:
    CameraViewport m_viewport = {};

    float m_fieldOfView;

    matrix4x4f m_translation;
    matrix4x4f m_lookAt;
    struct alignas(16) {
        matrix4x4f m_view;
        matrix4x4f m_projection;

        vector4f m_position;
        vector4f m_forward;
    };
    vector4f m_rotation;
public:
    Camera(CameraViewport viewport, float fieldOfView, vector4f position, vector4f rotation);
    Camera(CameraViewport viewport);

    void update(CameraInput& input, float deltaTime, float mouseSensitivity = 0.314f);

    void updateViewMatrix();
    matrix4x4f getViewMatrix();

    void updateProjectionMatrix();
    matrix4x4f getProjectionMatrix();

    void updateForwardVector();
    const vector4f& getForwardVector();

    CameraUniformBufferData* getShaderBufferPointer();

    const CameraViewport& getViewport() const { return m_viewport; }
    void setViewport(CameraViewport viewport) { m_viewport = viewport; }

    const float& getFieldOfView() const { return m_fieldOfView; }
    void setFieldOfView(float fieldOfView) { m_fieldOfView = fieldOfView; }

    const vector4f& getPosition() const { return m_position; }
    void setPosition(vector4f position) { m_position = position; }

    const vector4f& getRotation() const { return m_rotation; }
    void setRotation(vector4f rotation) { m_rotation = rotation; }

    static void cameraUIs();
};