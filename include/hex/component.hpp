#pragma once

#include <cstdint>
#include <string>

#include "codex/shader.hpp"

namespace hex {

// Forward declaration
class Actor;

#define ImplementComponentType(type) \
    public: \
        static ComponentTypeID getStaticID() { \
            static ComponentTypeID typeID = Component::getTypeID<type>(); \
            return typeID; \
        } \
        virtual ComponentTypeID getID() const override { \
            return getStaticID(); \
        }

class Component {
public:
    Component(Actor* actor) : m_actor(actor) { m_dependenciesFound = resolveDependencies(); }
    virtual ~Component() = default;

    using ComponentTypeID = std::uint32_t;
    
    template<typename _>
    static ComponentTypeID getTypeID() {
        static ComponentTypeID typeID = getNextTypeID();
        return typeID;
    }

    virtual ComponentTypeID getID() const = 0;
    virtual constexpr const std::string getPrettyName() const = 0;

    virtual void update() = 0;
    virtual void render(codex::Shader* overrideShader = nullptr) = 0;

    Actor* const getActor() const;

    inline void setEnabled(const bool enabled) { m_enabled = enabled; }
    inline bool isEnabled() const { return m_dependenciesFound && m_enabled; }

    /**
     * @brief Resolve dependencies for the component.
     * For example, a renderer component may need to resolve its transform component.
     *
     * @return true if dependencies were resolved successfully, false otherwise.
     * @note If this function returns false, the component will be disabled. 
     */
    virtual bool resolveDependencies() { return true; }
    virtual void onParentChanged() { m_dependenciesFound = resolveDependencies(); }
    virtual void editorUI();
protected:
    bool m_enabled = true;
    bool m_dependenciesFound = false;
    Actor* m_actor = nullptr;

    static ComponentTypeID getNextTypeID() {
        static ComponentTypeID lastID = 0;
        return lastID++;
    }
};

}; // namespace hex