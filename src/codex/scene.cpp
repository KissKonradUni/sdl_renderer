#include "codex/scene.hpp"

namespace Codex {

const std::array<std::string, TEXTURE_TYPE_COUNT> TextureTypeStrings = {
    "textureDiffuse",
    "textureNormal",
    "textureAORoughnessMetallic"
};

Drawable::Drawable(
    std::shared_ptr<Mesh>& mesh,
    std::shared_ptr<Shader>& shader,
    std::map<TextureType, std::shared_ptr<Texture>>& textures
) : m_texture(textures), m_shader(shader), m_mesh(mesh) {}

Drawable::~Drawable() {
    m_texture.clear();
    m_shader.reset();
    m_mesh.reset();
}

void Drawable::draw() {
    m_shader->bind();
    m_shader->setUniform(std::string("modelMatrix"), m_mesh->getModelMatrix());

    for (auto [type, texture] : m_texture) {
        texture->bind(static_cast<int>(type));
        m_shader->setUniform(TextureTypeStrings[type], static_cast<int>(type));
    }

    m_mesh->draw();
}

void Scene::draw() {
    for (const auto& drawable : m_drawables) {
        drawable->draw();
    }
}

}; // namespace Codex