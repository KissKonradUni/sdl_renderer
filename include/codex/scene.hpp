#pragma once

#include "mesh.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <memory>
#include <map>

namespace Codex {

#define TEXTURE_TYPE_COUNT 3
enum TextureType : uint8_t {
    DIFFUSE  = 0, // [rgba] Diffuse / Albedo
    NORMAL   = 1, // [rgb]  Normal map / Bump map
    AORM     = 2  // [r]    Ambient Occlusion, [g] Roughness, [b] Metallic
};

class Drawable {
public:
    Drawable(
        std::shared_ptr<Mesh> mesh,
        std::shared_ptr<Shader> shader,
        std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>> textures
    );
    ~Drawable();

    void draw();

    void setMesh(std::shared_ptr<Mesh>& mesh);
    void setShader(std::shared_ptr<Shader>& shader);
    void setTextures(std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>>& textures);

    std::shared_ptr<Mesh> getMesh() { return m_mesh; }
    std::shared_ptr<Shader> getShader() { return m_shader; }
    std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>>& getTextures() { return m_texture; }
protected:
    std::shared_ptr<std::map<TextureType, std::shared_ptr<Texture>>> m_texture;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Mesh> m_mesh;
};

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    void addDrawable(std::shared_ptr<Drawable> drawable);
    void draw();

    void sceneExplorerWindow();
    void inspectorWindow();
protected:
    std::vector<std::shared_ptr<Drawable>> m_drawables;

    std::shared_ptr<Drawable> m_selectedDrawable;
};

};