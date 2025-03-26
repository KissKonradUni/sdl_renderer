#include "hex/actor.hpp"
#include "imgui.h"

namespace hex {

Actor::Actor(Actor* parent) {
    m_parent = parent;
    if (m_parent != nullptr)
        m_parent->addChild(this);
    cinder::log("Actor created.");
}

Actor::~Actor() {
    m_children.clear();
    m_components.clear();
    cinder::log("Actor destroyed.");
}

void Actor::update() {
    if (!m_enabled) {
        return;
    }

    for (const auto& component : m_components) {
        if (component->isEnabled())
            component->update();
    }
}

void Actor::render() {
    if (!m_enabled) {
        return;
    }

    for (const auto& component : m_components) {
        if (component->isEnabled())
            component->render();
    }
}

void Actor::addChild(Actor* actor) {
    if (actor == nullptr) {
        cinder::warn("Cannot add null child to actor.");
        return;
    }

    m_children.push_back(actor);
    actor->setParent(this);
}

void Actor::removeChild(Actor* actor) {
    if (actor == nullptr) {
        cinder::warn("Cannot remove null child from actor.");
        return;
    }

    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (*it == actor) {
            m_children.erase(it);
            return;
        }
    }

    cinder::warn("Actor does not have the child to remove.");
}

void Actor::setParent(Actor* actor) {
    m_parent = actor;

    for (const auto& component : m_components) {
        component->onParentChanged();
    }
}

void Actor::editorUI() {    
    ImGui::Text("Actor %p", this);
    ImGui::Separator();

    auto availableSpace = ImGui::GetContentRegionAvail();
    for (const auto& component : m_components) {
        ImGui::BeginChild(component->getPrettyName().c_str(), ImVec2(availableSpace.x, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY);
        ImGui::Text("%s", component->getPrettyName().c_str());
        component->editorUI();
        ImGui::EndChild();
    }
}

}; // namespace hex