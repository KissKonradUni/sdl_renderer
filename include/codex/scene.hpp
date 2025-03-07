#pragma once

#include <SDL3/SDL.h>

namespace Codex {

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