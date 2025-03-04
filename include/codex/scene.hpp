#pragma once

#include <SDL3/SDL.h>

namespace Codex {

#define TEXTURE_TYPE_COUNT 3
enum TextureType : uint8_t {
    DIFFUSE  = 0, // [rgba] Diffuse / Albedo
    NORMAL   = 1, // [rgb]  Normal map / Bump map
    AORM     = 2  // [r]    Ambient Occlusion, [g] Roughness, [b] Metallic
};

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    void draw();

    void sceneExplorerWindow();
    void inspectorWindow();
protected:

};

};