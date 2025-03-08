#pragma once

#include "floatmath.hpp"

namespace Hex {

class Actor {
public:
    Actor() = default;
    Actor(transformf* parent) : m_transform(parent) {}
protected:
    transformf m_transform;
};

}; // namespace Hex