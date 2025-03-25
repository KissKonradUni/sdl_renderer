#pragma once

#include <cstdint>
#include <string>

namespace Hex {

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
    Component(Actor* actor) : m_actor(actor) {}
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
    virtual void render() = 0;

    Actor* const getActor() const;

    void setEnabled(const bool enabled);
    inline bool isEnabled() const { return m_enabled; }

    virtual void onParentChanged() {}
    virtual void editorUI();
protected:
    bool m_enabled = true;
    Actor* m_actor = nullptr;

    static ComponentTypeID getNextTypeID() {
        static ComponentTypeID lastID = 0;
        return lastID++;
    }
};

}; // namespace Hex