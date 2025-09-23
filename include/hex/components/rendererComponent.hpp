#pragma once

#include "codex/mesh.hpp"
#include "codex/shader.hpp"
#include "codex/material.hpp"

#include "hex/component.hpp"
#include "hex/components/transformComponent.hpp"

namespace hex {

class RendererComponent : public Component {
    ImplementComponentType(RendererComponent)
public:
    RendererComponent(Actor* actor);
    RendererComponent(Actor* actor, codex::Shader* shader, codex::Material* material, codex::Mesh* mesh);
    virtual ~RendererComponent() = default;

    constexpr const std::string getPrettyName() const override { return "Renderer"; }

    void update() override;
    void render(codex::Shader* overrideShader = nullptr) override;

    virtual bool resolveDependencies() override;
    virtual void onParentChanged() override;
    virtual void editorUI() override;

    inline void setShader  (codex::Shader*   shader  ) { m_shader   = shader;   }
    inline void setMaterial(codex::Material* material) { m_material = material; }
    inline void setMesh    (codex::Mesh*     mesh    ) { m_mesh     = mesh;     }

    inline codex::Shader*   getShader()   const { return m_shader;   }
    inline codex::Material* getMaterial() const { return m_material; }
    inline codex::Mesh*     getMesh()     const { return m_mesh;     }
protected:
    hex::TransformComponent* m_transformComponent = nullptr;

    codex::Shader*   m_shader   = nullptr;
    codex::Material* m_material = nullptr;
    codex::Mesh*     m_mesh     = nullptr;
};

}; // namespace hex