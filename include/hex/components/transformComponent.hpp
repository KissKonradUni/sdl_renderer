#pragma once

#include "hex/component.hpp"
#include "floatmath.hpp"

namespace Hex {

class TransformComponent : public Component {
    ImplementComponentType(TransformComponent)
public:
    TransformComponent(Actor* actor);
    virtual ~TransformComponent() = default;

    constexpr const std::string getPrettyName() const override { return "Transform"; }

    void update() override;
    void render() override;

    inline transformf& getTransform() { return m_transform; }

    virtual void onParentChanged() override;
    virtual void editorUI() override;
protected:
    transformf m_transform;
};

}; // namespace Hex