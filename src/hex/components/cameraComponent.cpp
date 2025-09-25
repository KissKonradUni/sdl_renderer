#include "hex/components/cameraComponent.hpp"
#include "hex/actor.hpp"
#include "glad.h"

namespace hex {

static unsigned int UNIFORM_BINDING_POINT = 0;

CameraComponent::CameraComponent(Actor* actor, CameraViewport viewport, float fov, vector4f position, vector4f rotation) : Component(actor) {
    m_camera = std::make_unique<Camera>(viewport, fov, position, rotation);
    m_cameraUniformBuffer = std::make_unique<codex::UniformBuffer>(sizeof(CameraUniformBufferData), UNIFORM_BINDING_POINT++);

    m_dependenciesFound = true;
}

CameraComponent::CameraComponent(Actor* actor, float left, float right, float bottom, float top, float nearPlane, float farPlane) : Component(actor) {
    m_camera = std::make_unique<Camera>(left, right, bottom, top, nearPlane, farPlane);
    m_cameraUniformBuffer = std::make_unique<codex::UniformBuffer>(sizeof(CameraUniformBufferData), UNIFORM_BINDING_POINT++);

    m_dependenciesFound = true;
}

void CameraComponent::update() {

}

void CameraComponent::render(codex::Shader* overrideShader) {
    if (m_camera->isOrtographic())
        return; // TODO: temporary fix, make a better solution later

    if (m_cameraUniformBuffer)
        m_cameraUniformBuffer->updateData(m_camera->getShaderBufferPointer());
    else 
        cinder::warn("Camera component missing uniform buffer.");
}

void CameraComponent::resizeCamera(float width, float height) {
    m_camera->setViewport({0, 0, width, height});
    m_camera->updateProjectionMatrix();
    glViewport(0, 0, width, height);
}

void CameraComponent::sendDebugCameraInput(float deltaTime) {
    m_camera->update(m_cameraInput, deltaTime);
}

void CameraComponent::editorUI() {
    m_camera->cameraWindow(false);
}

}