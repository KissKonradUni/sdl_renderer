#pragma once

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <memory>
#include <map>

namespace Codex {

#define TEXTURE_TYPE_COUNT 3
enum TextureType : uint8_t {
    DIFFUSE  = 0,
    NORMAL   = 1,
    // Ambient Occlusion, Roughness, Metallic
    AORM     = 2
};

class Drawable {
public:
    Drawable(
        std::shared_ptr<Mesh>& mesh,
        std::shared_ptr<Shader>& shader,
        std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>>& textures
    );
    ~Drawable();

    void draw();

    void setMesh(std::shared_ptr<Mesh>& mesh) { m_mesh = mesh; }
    void setShader(std::shared_ptr<Shader>& shader) { m_shader = shader; }
    void setTextures(std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>>& textures) { m_texture = textures; }
protected:
    std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>> m_texture;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Mesh> m_mesh;
};

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    void addDrawable(std::shared_ptr<Drawable> drawable) { m_drawables.push_back(drawable); }
    void draw();

protected:
    std::vector<std::shared_ptr<Drawable>> m_drawables;
};

};