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
protected:
    transformf m_transform;
};

}; // namespace Hex