#include "hex/components/transformComponent.hpp"
#include "hex/actor.hpp"

#include "imgui.h"

namespace hex {

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
        m_transform.setParent(parentTransform->getTransformPtr());
    }
}

void TransformComponent::update() {
    // Nothing to do
}

void TransformComponent::render() {
    // Nothing to do
}

void TransformComponent::editorUI() {    
    if (!ImGui::BeginTable("##transform_props", 2, ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableNextColumn();
    ImGui::Text("Position: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);    
    if (ImGui::InputFloat3("Position", &m_transform.unsafe_getPosition()->x)) {;
        m_transform.markDirty();
    }

    ImGui::TableNextColumn();
    ImGui::Text("Rotation: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    if (ImGui::InputFloat3("Rotation", &m_transform.unsafe_getRotation()->x)) {
        m_transform.markDirty();
    }

    ImGui::TableNextColumn();
    ImGui::Text("Scale: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    if (ImGui::InputFloat3("Scale", &m_transform.unsafe_getScale()->x)) {
        m_transform.markDirty();
    }

    ImGui::EndTable();

    ImGui::Separator();
    ImGui::Text("Pointer: %p", this);
}

}; // namespace hex