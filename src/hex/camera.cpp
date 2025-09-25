#include "hex/camera.hpp"

#include "imgui.h"

namespace hex {

Camera::Camera(CameraViewport viewport, float fieldOfView, vector4f position, vector4f rotation) {
    this->m_viewport = viewport;
    this->m_fieldOfView = fieldOfView;

    this->m_position = position;
    this->m_rotation = rotation;
    
    this->updateProjectionMatrix();
}

Camera::Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    this->m_isOrthographic = true;
    this->m_viewport = {0.0f, 0.0f, right - left, top - bottom};
    this->m_fieldOfView = 0.0f;

    this->m_position = vector4f::zero();
    this->m_rotation = vector4f::zero();

    this->m_projection = matrix4x4f::orthographic(left, right, bottom, top, nearPlane, farPlane);
}

Camera::Camera(CameraViewport viewport): Camera(viewport, 80.0f, vector4f::zero(), vector4f::zero()) {}

void Camera::update(CameraInput& input, float deltaTime, float mouseSensitivity) {
    if (input.lock)
        return;

    const auto forward = vector4f(input.movement.x, 0.0f, input.movement.y, 0.0f).normalize3d();
    const auto rotate  = matrix4x4f::rotation(this->m_rotation.y, vector4f::up());
          auto move    = (rotate * (forward * deltaTime * 2.0f)).normalize3d() * 10.0f * deltaTime;
               move.y  = input.movement.z * deltaTime * 2.0f * 10.0f;

    this->m_position   = this->m_position + move;
    this->m_rotation.x = SDL_clamp(this->m_rotation.x - input.rotation.yaw * deltaTime * 0.314f, -SDL_PI_F / 2.0f, SDL_PI_F / 2.0f);
    this->m_rotation.y = this->m_rotation.y - input.rotation.pitch * deltaTime * mouseSensitivity;

    if (this->m_rotation.y >  SDL_PI_F) this->m_rotation.y -= 2.0f * SDL_PI_F;
    if (this->m_rotation.y < -SDL_PI_F) this->m_rotation.y += 2.0f * SDL_PI_F;
    input.rotation.yaw   = 0.0f;
    input.rotation.pitch = 0.0f;
}

void Camera::updateViewMatrix() {
    this->m_translation = matrix4x4f::translation(this->m_position * -1.0f);
    this->m_lookAt      = matrix4x4f::lookAt(this->m_rotation);
    this->m_view        = this->m_translation * this->m_lookAt;
}

matrix4x4f Camera::getViewMatrix() {
    updateViewMatrix();

    return this->m_view;
}

void Camera::updateProjectionMatrix() {
    if (this->m_isOrthographic)
        return; // TODO: make it changeable

    this->m_projection = matrix4x4f::perspective(
        this->m_fieldOfView * (SDL_PI_F / 180.0f), 
        this->m_viewport.w / this->m_viewport.h, 
        0.5f,   
        100.0f //TODO: make configurable
    );
}

matrix4x4f Camera::getProjectionMatrix() {
    updateProjectionMatrix();

    return this->m_projection;
}

void Camera::updateForwardVector() {
    m_forward = vector4f::front() * m_lookAt;
}

const vector4f& Camera::getForwardVector() {
    updateForwardVector();
    return m_forward;
}

CameraUniformBufferData* Camera::getShaderBufferPointer() {
    updateViewMatrix();
    updateForwardVector();

    return &m_shaderBufferData;
}

void Camera::cameraWindow(bool separateWindow) {
    if (separateWindow)
        ImGui::Begin("Camera", nullptr);
    
    if (!ImGui::BeginTable("##cam_props", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::End();
        return;
    }

    ImGui::TableNextColumn();
    ImGui::Text("Position: ");

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    ImGui::InputFloat3("##cam_pos", &this->m_position.x);

    ImGui::TableNextColumn();
    ImGui::Text("Rotation: ");

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    ImGui::InputFloat2("##cam_rot", &this->m_rotation.x);

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    ImGui::Text("Fov: ");
    
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    if (ImGui::InputFloat("##cam_fov", &this->m_fieldOfView, 0.0f, 0.0f)) {
        this->m_fieldOfView = SDL_clamp(this->m_fieldOfView, 1.0f, 179.0f);
        this->updateProjectionMatrix();
    }

    ImGui::TableNextColumn();
    ImGui::Text("Viewport: ");
    
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    ImGui::InputFloat2("##cam_viewport", &this->m_viewport.w);

    ImGui::EndTable();

    if (separateWindow)
        ImGui::End();
}

}; // namespace hex