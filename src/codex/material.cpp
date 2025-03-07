#include "codex/material.hpp"
#include "lib/json/json.hpp"
#include "codex/assets.hpp"
#include "echo/console.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <array>

namespace Codex {

const std::array<std::string, TEXTURE_TYPE_COUNT> TextureTypeStrings = {
    "textureDiffuse",
    "textureNormal",
    "textureAORoughnessMetallic"
};

Material::Material(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        Echo::error(std::string("Material file not found: ") + path);
        return;
    }

    using nlohmann::json;

    std::ifstream file(path);
    json data = json::parse(file);

    m_name = data["name"].template get<std::string>();

    auto texturesObject = data["textures"];
    if (texturesObject.is_null()) {
        Echo::warn("Material missing textures object.");
        return;
    }

    if (texturesObject.contains(TextureTypeStrings[DIFFUSE])) {
        m_diffusePath = texturesObject[TextureTypeStrings[DIFFUSE]].template get<std::string>();
        Assets::instance().getTextureLibrary().getAsync(m_diffusePath, [this](std::shared_ptr<Texture> texture) {
            m_diffuse = texture;
            m_diffuse->m_node = Assets::instance().findAsset(m_diffusePath);
        });
    } else {
        Echo::warn("Material missing diffuse texture.");
    }

    if (texturesObject.contains(TextureTypeStrings[NORMAL])) {
        m_normalPath = texturesObject[TextureTypeStrings[NORMAL]].template get<std::string>();
        Assets::instance().getTextureLibrary().getAsync(m_normalPath, [this](std::shared_ptr<Texture> texture) {
            m_normal = texture;
            m_normal->m_node = Assets::instance().findAsset(m_normalPath);
        });
    } else {
        Echo::warn("Material missing normal texture.");
    }

    if (texturesObject.contains(TextureTypeStrings[AORM])) {
        m_aormPath = texturesObject[TextureTypeStrings[AORM]].template get<std::string>();
        Assets::instance().getTextureLibrary().getAsync(m_aormPath, [this](std::shared_ptr<Texture> texture) {
            m_aorm = texture;
            m_aorm->m_node = Assets::instance().findAsset(m_aormPath);
        });
    } else {
        Echo::warn("Material missing AORoughnessMetallic texture.");
    }

    Echo::log(std::string("Material created: ") + m_name);
}

Material::~Material() {
    Echo::log(std::string("Material destroyed: ") + m_name);
}

void Material::bindTextures() const {
    if (m_diffuse) {
        m_diffuse->bind(0);
    }
    if (m_normal) {
        m_normal->bind(1);
    }
    if (m_aorm) {
        m_aorm->bind(2);
    }
}

}; // namespace Codex