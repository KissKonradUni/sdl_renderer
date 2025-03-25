#include "hex/components/transformComponent.hpp"
#include "hex/actor.hpp"

#include "imgui.h"

namespace Hex {

TransformComponent::TransformComponent(Actor* actor) : Component(actor), m_transform() {
    onParentChanged();
}

void TransformComponent::onParentChanged() {
    auto parent = m_actor->getParent();
    if (parent == nullptr) {
        m_transform.setParent(nullptr);
        return;
    }
    
    auto parentTransform = parent->getComponent<TransformComponent>(true);
    if (parentTransform != nullptr) {
        m_transform = parentTransform->getTransform();
    }
}

void TransformComponent::update() {
    // Nothing to do
}

void TransformComponent::render() {
    // Nothing to do
}

void TransformComponent::editorUI() {
    auto position = m_transform.getPosition();
    ImGui::InputFloat3("Position", &position.x);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        m_transform.setPosition(position);
    }

    auto rotation = m_transform.getRotation();
    ImGui::InputFloat3("Rotation", &rotation.x);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        m_transform.setRotation(rotation);
    }

    auto scale = m_transform.getScale();
    ImGui::InputFloat3("Scale", &scale.x);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        m_transform.setScale(scale);
    }
}

}; // namespace Hex