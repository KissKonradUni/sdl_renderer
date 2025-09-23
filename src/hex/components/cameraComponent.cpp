#include "hex/components/cameraComponent.hpp"
#include "hex/actor.hpp"
#include "glad.h"

namespace hex {

CameraComponent::CameraComponent(Actor* actor, CameraViewport viewport, float fov, vector4f position, vector4f rotation) : Component(actor) {
    m_camera = std::make_unique<Camera>(viewport, fov, position, rotation);
    m_cameraUniformBuffer = std::make_unique<codex::UniformBuffer>(sizeof(CameraUniformBufferData), 0);

    m_dependenciesFound = true;
}

void CameraComponent::update() {

}

void CameraComponent::render(codex::Shader* overrideShader) {
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