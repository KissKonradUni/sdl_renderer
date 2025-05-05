#include "hex/components/rendererComponent.hpp"
#include "hex/actor.hpp"

#include "cinder.hpp"
#include "imgui.h"

namespace hex {

RendererComponent::RendererComponent(Actor* actor, codex::Shader* shader, codex::Material* material, codex::Mesh* mesh) : Component(actor) {
    m_shader   = shader;
    m_material = material;
    m_mesh     = mesh;

    m_dependenciesFound = resolveDependencies();
}

RendererComponent::RendererComponent(Actor* actor) : hex::RendererComponent(actor, nullptr, nullptr, nullptr) {
    // Nothing to do here
}

void RendererComponent::update() {
    // Nothing to do here
}

void RendererComponent::render() {
    if (m_shader == nullptr || m_material == nullptr || m_mesh == nullptr) {
        return;
    }

    if (!m_shader->isInitialized() || !m_material->isInitialized() || !m_mesh->isInitialized()) {
        return;
    }

    m_shader->bind();
    m_material->bindTextures(m_shader);

    if (m_transformComponent == nullptr) {
        cinder::warn("Renderer component requires a transform component.");
        return;
    }

    m_shader->setUniform("modelMatrix", m_transformComponent->getTransform().getModelMatrix());
    m_mesh->draw();
}

bool RendererComponent::resolveDependencies() {
    cinder::log("Resolving dependencies for renderer component.");
    m_transformComponent = m_actor->getComponent<TransformComponent>();
    if (m_transformComponent == nullptr) {
        cinder::warn("Renderer component requires a transform component.");
        return false;
    }
    return true;
}

void RendererComponent::onParentChanged() {
    m_dependenciesFound = resolveDependencies();
}

void RendererComponent::editorUI() {
    if (!ImGui::BeginTable("##transform_props", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInner)) {
        return;
    }

    ImGui::TableNextColumn();
    ImGui::Text("Shader: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);    
    if (m_shader != nullptr && m_shader->getNode() != nullptr) {
        if (ImGui::Selectable(m_shader->getNode()->name.c_str(), true)) {
            // TODO: Implement file browser
        }
    }

    ImGui::TableNextColumn();
    ImGui::Text("Material: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    if (m_material != nullptr && m_material->getNode() != nullptr) {    
        if (ImGui::Selectable(m_material->getNode()->name.c_str(), true)) {
            // TODO: Implement file browser
        }
    }

    ImGui::TableNextColumn();
    ImGui::Text("Mesh: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-0.001f);
    if (m_mesh != nullptr && m_mesh->getNode() != nullptr) {
        if (ImGui::Selectable(m_mesh->getNode()->name.c_str(), true)) {
            // TODO: Implement file browser
        }
    }

    ImGui::EndTable();
}

}