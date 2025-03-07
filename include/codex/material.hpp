#pragma once

#include "codex/resource.hpp"
#include "codex/texture.hpp"

#include <cstdint>

namespace Codex {

#define TEXTURE_TYPE_COUNT 3
enum TextureType : uint8_t {
    DIFFUSE  = 0, // [rgba] Diffuse / Albedo
    NORMAL   = 1, // [rgb]  Normal map / Bump map
    AORM     = 2  // [r]    Ambient Occlusion, [g] Roughness, [b] Metallic
};

struct MaterialData {
    std::string path;
};

class Material : public IResource {
friend class Assets;
public:
    Material(const std::string& path);
    ~Material();

    void bindTextures() const;
private:
    std::string m_name;

    std::string m_diffusePath;
    std::string m_normalPath;
    std::string m_aormPath;

    std::shared_ptr<Texture> m_diffuse;
    std::shared_ptr<Texture> m_normal;
    std::shared_ptr<Texture> m_aorm;
};

}; // namespace Codex