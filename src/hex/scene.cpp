#include "hex/scene.hpp"

#include "imgui.h"
#include <IconsMaterialSymbols.h>

namespace hex {

Scene::Scene() {
    
}

Scene::~Scene() {
    
}

void Scene::update() {
    for (const auto& actor : m_actors) {
        actor->update();
    }
}

void Scene::render() {
    for (const auto& actor : m_actors) {
        actor->render();
    }
}

Actor* Scene::newActor() {
    auto actor = new Actor();
    m_actors.push_back(std::unique_ptr<Actor>(actor));
    return actor;
}

void Scene::removeActor(Actor* actor) {
    for (auto it = m_actors.begin(); it != m_actors.end(); ++it) {
        if (it->get() == actor) {
            m_actors.erase(it);
            return;
        }
    }
}

char buffer[64];
void Scene::drawActorTree(Actor* actor, int depth) {
    if (actor == nullptr) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        ImGui::Text(" ");
        ImGui::SameLine();
    }

    auto children = actor->getChildren();
    if (!children.empty()) {
        auto expanded = actor->isEditorExpanded();
        snprintf(buffer, 64, "%s##%p", expanded ? ICON_MS_KEYBOARD_ARROW_DOWN : ICON_MS_KEYBOARD_ARROW_RIGHT, actor);
        if (ImGui::Button(buffer)) {
            actor->setEditorExpanded(!expanded);
        }
    } else {
        ImGui::Text("   ");
    }
    ImGui::SameLine();

    snprintf(buffer, 64, "%s##%p", actor->isEnabled() ? ICON_MS_CHECK_BOX : ICON_MS_CHECK_BOX_OUTLINE_BLANK, actor);
    if (ImGui::Button(buffer)) {
        actor->setEnabled(!actor->isEnabled());
    }
    ImGui::SameLine();

    snprintf(buffer, 64, "%s##%p", actor->getName().c_str(), actor);
    if (ImGui::Button(buffer, ImVec2(-0.1f, 0))) {
        m_selectedActor = actor;
    }

    if (!actor->isEditorExpanded())
        return;

    for (const auto& child : children) {
        drawActorTree(child, depth + 1);
    }
}

void Scene::editorUI() {
    ImGui::Begin("Scene", nullptr);

    auto availableSpace = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("Tree", ImVec2(0, availableSpace.y * 0.4f), ImGuiChildFlags_Border);

    for (const auto& actor : m_actors) {
        if (actor->getParent() == nullptr)
            drawActorTree(actor.get());
    }

    ImGui::EndChild();

    ImGui::BeginChild("Properties", ImVec2(0, 0), ImGuiChildFlags_Border);

    if (m_selectedActor == nullptr) {
        ImGui::Text("No actor selected.");
    } else {
        m_selectedActor->editorUI();
    }

    ImGui::EndChild();

    ImGui::End();
}

}; // namespace hex