#pragma once

#include "console.hpp"

#include <memory>
#include <map>

#include <SDL3/SDL.h>

typedef void (*UIFunction)();

class Imgui_UIManager {
public:
    SDL_AppResult initUI();
    void processEvent(SDL_Event* event);
    void newFrame();
    void render();
    void cleanup();

    unsigned int addUIFunction(UIFunction uiFunction);
    void removeUIFunction(unsigned int id);
protected:
    std::map<unsigned int, UIFunction> uiFunctions;
};

class Imgui_UIManagerDeleter {
public:
    void operator()(Imgui_UIManager* uiManager) {
        uiManager->cleanup();
        console->log("ImGui closed.");
        console->log("UIManager destroyed.");
        delete uiManager;
    }
};

extern std::unique_ptr<Imgui_UIManager, Imgui_UIManagerDeleter> UIManager;