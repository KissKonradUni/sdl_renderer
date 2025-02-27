#include "codex/scene.hpp"
#include "codex/assets.hpp"

#include "imgui.h"

namespace Codex {

const std::array<std::string, TEXTURE_TYPE_COUNT> TextureTypeStrings = {
    "textureDiffuse",
    "textureNormal",
    "textureAORoughnessMetallic"
};

Drawable::Drawable(
    std::shared_ptr<Mesh> mesh,
    std::shared_ptr<Shader> shader,
    std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>> textures
) : m_texture(textures), m_shader(shader), m_mesh(mesh) {}

Drawable::~Drawable() {
    m_texture.reset();
    m_shader.reset();
    m_mesh.reset();
}

void Drawable::draw() {
    if (!m_shader)
        return;

    if (!m_mesh)
        return;

    if (!m_texture)
        return;

    m_shader->bind();
    m_shader->setUniform(std::string("modelMatrix"), m_mesh->transform.getModelMatrix());

    for (auto [type, texture] : *m_texture) {
        texture->bind(static_cast<int>(type));
        m_shader->setUniform(TextureTypeStrings[type], static_cast<int>(type));
    }

    m_mesh->draw();
}

void Drawable::setMesh(std::shared_ptr<Mesh>& mesh) {
    m_mesh = mesh;
}

void Drawable::setShader(std::shared_ptr<Shader>& shader) {
    m_shader = shader;
}

void Drawable::setTextures(std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>>& textures) {
    m_texture = textures;
}

void Scene::draw() {
    for (const auto& drawable : m_drawables) {
        drawable->draw();
    }
}

void Scene::addDrawable(std::shared_ptr<Drawable> drawable) {
    m_drawables.push_back(drawable);
}

void Scene::sceneExplorerWindow() {
    ImGui::Begin("Scene");

    auto windowSize = ImGui::GetWindowContentRegionMax();

    for (int i = 0; i < m_drawables.size(); i++) {
        std::string name = "Drawable##" + std::to_string(i);
        if (ImGui::Button(name.c_str(), ImVec2(windowSize.x - 8, 0))) {
            m_selectedDrawable = m_drawables[i];
        }
    }

    ImGui::End();
}

void Scene::inspectorWindow() {
    ImGui::Begin("Inspector");

    if (m_selectedDrawable && m_selectedDrawable->getMesh()) {
        ImGui::Text("Selected Drawable");

        // TODO: Add information about assets.
        // Right now the mesh, shader and textures do not include any information after they are loaded.
        ImGui::Indent();
        ImGui::Text("Mesh: %p",     m_selectedDrawable->getMesh().get());
        ImGui::Text("Shader: %p",   m_selectedDrawable->getShader().get());
        ImGui::Text("Textures: %p", m_selectedDrawable->getTextures().get());
        ImGui::Unindent();
        
        auto transform = &m_selectedDrawable->getMesh()->transform;
        vector4f position = transform->getPosition();
        ImGui::InputFloat3("##position", &position.x);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            transform->setPosition(position);
        }

        vector4f rotation = transform->getRotation();
        ImGui::InputFloat3("##rotation", &rotation.x);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            transform->setRotation(rotation);
        }

        vector4f scale = transform->getScale();
        ImGui::InputFloat3("##scale", &scale.x);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            transform->setScale(scale);
        }    
    } else {
        ImGui::Text("No drawable selected.");
    }

    ImGui::End();
}

}; // namespace Codex