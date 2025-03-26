#include "codex/material.hpp"
#include "codex/library.hpp"
#include "echo/console.hpp"

#include <json.hpp>

#include <fstream>

namespace codex {

Material::Material(std::string& name, std::map<std::string, Texture*>& textures) {
    m_data.reset(new MaterialData{name, textures});
    loadResource();
    m_data.reset();

    m_runtimeResource = true;
    m_node = nullptr;

    echo::log("Runtime material created.");
}

Material::Material() {
    echo::log("Material placeholder created.");
}

Material::~Material() {
    echo::log("Material destroyed.");
}

void Material::loadData(const FileNode* file) {
    if (m_initialized) {
        echo::warn("Material already initialized.");
        return;
    }

    if (file == nullptr) {
        echo::error("No file to load material from.");
        return;
    }

    // Load the material data from the file
    using namespace nlohmann;
    auto& library = codex::Library::instance();

    std::ifstream metaFile(library.getAssetsRoot() / file->path);
    json meta = json::parse(metaFile);

    m_node = file;
    m_data.reset(new MaterialData);
    m_data->name = meta["name"].template get<std::string>();

    if (meta["textures"].is_null()) {
        echo::warn("No textures found in material file.");
        return;
    }

    for (auto it = meta["textures"].begin(); it != meta["textures"].end(); ++it) {
        std::string name = it.key();
        std::filesystem::path texturePath = it.value().template get<std::string>();
        library.formatPath(&texturePath);

        auto textureNode = library.tryGetAssetNode(texturePath);
        if (textureNode == nullptr) {
            echo::warn("Texture not found: " + texturePath.string());
            continue;
        }
        auto texture = library.tryLoadResource<Texture>(textureNode);
        if (texture == nullptr) {
            echo::warn("Texture could not be loaded: " + texturePath.string());
            continue;
        }

        m_data->textures[name] = texture;
    }

    m_runtimeResource = false;
    echo::log("Loaded material data from file: " + file->path.string());
}

void Material::loadResource() {
    if (m_initialized) {
        echo::warn("Material already initialized.");
        return;
    }

    if (m_data == nullptr) {
        echo::error("Material data is null.");
        return;
    }

    m_initialized = true;
}

void Material::bindTextures(Shader* shader) const {
    if (!m_initialized) {
        echo::warn("Material not initialized.");
        return;
    }

    int index = 0;
    for (const auto& [name, texture] : m_data->textures) {
        texture->bind(index);
        shader->setUniform(name, index);
        index++;
    }
}

void Material::setTexture(const std::string& name, Texture* texture) {
    if (!m_initialized) {
        echo::warn("Material not initialized.");
        return;
    }

    m_data->textures[name] = texture;
}

void Material::removeTexture(const std::string& name) {
    if (!m_initialized) {
        echo::warn("Material not initialized.");
        return;
    }

    m_data->textures.erase(name);
}

const Texture* Material::getTexture(const std::string& name) const {
    if (!m_initialized) {
        echo::warn("Material not initialized.");
        return nullptr;
    }

    if (m_data->textures.find(name) == m_data->textures.end()) {
        echo::warn("Texture not found in material.");
        return nullptr;
    }

    return m_data->textures[name];
}

}; // namespace codex