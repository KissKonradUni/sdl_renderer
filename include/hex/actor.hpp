#pragma once

#include <memory>
#include <vector>

#include "echo/console.hpp"
#include "hex/component.hpp"

namespace Hex {

class Actor {
public:
    Actor(Actor* parent = nullptr);
    ~Actor();

    // TODO: Add explicit deep copy
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    void update();
    void render();

    template<typename ComponentType = Component, typename... Args>
    bool addComponent(Args&&... args) {
        for (const auto& component : m_components) {
            if (component->getID() == ComponentType::getStaticID()) {
                Echo::warn("Actor already has component of type.");
                return false;
            }
        }

        m_components.push_back(std::make_unique<ComponentType>(this, std::forward<Args>(args)...));
        return true;
    }
    
    template<typename ComponentType = Component>
    bool removeComponent() {
        for (auto it = m_components.begin(); it != m_components.end(); ++it) {
            if ((*it)->getID() == ComponentType::getStaticID()) {
                m_components.erase(it);
                return true;
            }
        }

        Echo::warn("Actor does not have component of requested type.");
        return false;
    }
    
    template<typename ComponentType = Component>
    ComponentType* getComponent() const {
        for (const auto& component : m_components) {
            if (component->getID() == ComponentType::getStaticID()) {
                return static_cast<ComponentType*>(component.get());
            }
        }

        Echo::warn("Actor does not have component of requested type.");
        return nullptr;
    }

    void addChild(Actor* actor);
    void removeChild(Actor* actor);

    inline void setParent(Actor* actor) { m_parent = actor; }
    inline Actor* const getParent() const { return m_parent; }

    inline void setEnabled(const bool enabled) { m_enabled = enabled; }
    inline bool isEnabled() const { return m_enabled; }
protected:
    bool m_enabled = true;

    Actor* m_parent = nullptr;
    std::vector<Actor*> m_children;

    std::vector<std::unique_ptr<Component>> m_components;
};

}; // namespace Hex