#include "hex/actor.hpp"

namespace Hex {

Actor::Actor(Actor* parent) {
    m_parent = parent;
    Echo::log("Actor created.");
}

Actor::~Actor() {
    m_children.clear();
    m_components.clear();
    Echo::log("Actor destroyed.");
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
        Echo::warn("Cannot add null child to actor.");
        return;
    }

    m_children.push_back(actor);
    actor->setParent(this);
}

void Actor::removeChild(Actor* actor) {
    if (actor == nullptr) {
        Echo::warn("Cannot remove null child from actor.");
        return;
    }

    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (*it == actor) {
            m_children.erase(it);
            return;
        }
    }

    Echo::warn("Actor does not have the child to remove.");
}

}; // namespace Hex