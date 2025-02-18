#include "camera.hpp"

#include "imgui.h"
#include "ui.hpp"

#include <vector>

std::vector<Camera*> m_cameras = {};
bool m_uiRegistered = false;

Camera::Camera(CameraViewport viewport, float fieldOfView, vector4f position, vector4f rotation) {
    this->m_viewport = viewport;
    this->m_fieldOfView = fieldOfView;

    this->m_position = position;
    this->m_rotation = rotation;
    
    this->updateProjectionMatrix();

    m_cameras.push_back(this);
    if (!m_uiRegistered) {
        Echo::UI::instance().addUIFunction(cameraUIs);
        m_uiRegistered = true;
    }
}

Camera::Camera(CameraViewport viewport): Camera(viewport, 80.0f, vector4f::zero(), vector4f::zero()) {}

void Camera::update(CameraInput& input, float deltaTime, float mouseSensitivity) {
    if (input.lock)
        return;

    const auto forward = vector4f(input.movement.x, 0.0f, input.movement.y, 0.0f).normalize3d();
    const auto rotate  = matrix4x4f::rotation(this->m_rotation.y, vector4f::up());
    const auto move    = (rotate * (forward * deltaTime * 2.0f)).normalize3d() * 10.0f * deltaTime;

    this->m_position   = this->m_position + move;
    this->m_rotation.x = SDL_clamp(this->m_rotation.x - input.rotation.yaw * deltaTime * 0.314f, -SDL_PI_F / 2.0f, SDL_PI_F / 2.0f);
    this->m_rotation.y = this->m_rotation.y - input.rotation.pitch * deltaTime * mouseSensitivity;

    if (this->m_rotation.y >  SDL_PI_F) this->m_rotation.y -= 2.0f * SDL_PI_F;
    if (this->m_rotation.y < -SDL_PI_F) this->m_rotation.y += 2.0f * SDL_PI_F;
    input.rotation.yaw   = 0.0f;
    input.rotation.pitch = 0.0f;
}

void Camera::updateViewMatrix() {
    this->m_translation = matrix4x4f::translation(this->m_position);
    this->m_lookAt      = matrix4x4f::lookAt(this->m_rotation);
    this->m_view        = this->m_translation * this->m_lookAt;
}

matrix4x4f Camera::getViewMatrix() {
    updateViewMatrix();

    return this->m_view;
}

void Camera::updateProjectionMatrix() {
    this->m_projection = matrix4x4f::perspective(
        this->m_fieldOfView * (SDL_PI_F / 180.0f), 
        this->m_viewport.w / this->m_viewport.h, 
        0.5f,   
        100.0f
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

    return reinterpret_cast<CameraUniformBufferData*>(&m_view);
}

void Camera::cameraUIs() {
    int i = 1;
    for (auto& camera : m_cameras) {
        std::string name = "Camera " + std::to_string(i++);
        ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoBackground);
        
        ImGui::Text("Position: ");
        ImGui::SameLine();
        ImGui::InputFloat3("##cam_pos", &camera->m_position.x);

        ImGui::Text("Rotation: ");
        ImGui::SameLine();
        ImGui::InputFloat2("##cam_rot", &camera->m_rotation.x);

        ImGui::Text("Fov:      ");
        ImGui::SameLine();
        if (ImGui::InputFloat("##cam_fov", &camera->m_fieldOfView, 0.0f, 0.0f)) {
            camera->m_fieldOfView = SDL_clamp(camera->m_fieldOfView, 1.0f, 179.0f);
            camera->updateProjectionMatrix();
        }

        ImGui::Text("Viewport: ");
        ImGui::SameLine();
        ImGui::InputFloat2("##cam_viewport", &camera->m_viewport.w);

        ImGui::End();
    }
}