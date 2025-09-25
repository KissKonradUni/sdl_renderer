#pragma once

#include "hex/camera.hpp"
#include "hex/component.hpp"

namespace hex {

class CameraComponent : public Component {
    ImplementComponentType(CameraComponent)
public:
    CameraComponent(Actor* actor, CameraViewport viewport = CameraViewport{0,0,1280,720}, float fov = 80.0f, vector4f position = vector4f::zero(), vector4f rotation = vector4f::zero());
    CameraComponent(Actor* actor, float left, float right, float bottom, float top, float nearPlane, float farPlane);
    virtual ~CameraComponent() = default;

    constexpr const std::string getPrettyName() const override { return "Camera"; }

    void update() override;
    void render(codex::Shader* overrideShader = nullptr) override;

    inline CameraInput* getCameraInput() { return &m_cameraInput; }
    inline Camera* getCamera() const { return m_camera.get(); }

    void resizeCamera(float width, float height);
    void sendDebugCameraInput(float deltaTime);

    virtual void editorUI() override;
protected:
    std::unique_ptr<Camera> m_camera = nullptr;
    std::unique_ptr<codex::UniformBuffer> m_cameraUniformBuffer = nullptr;
    CameraInput m_cameraInput;
};

}; // namespace hex