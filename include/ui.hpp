#pragma once

#include <map>

#include <SDL3/SDL.h>

typedef void (*UIFunction)();

namespace Echo {

class UI {
public:
    static UI& instance() {
        static UI ui;
        return ui;
    }

    SDL_AppResult initUI();
    void processEvent(SDL_Event* event);
    void newFrame();
    void render();
    void cleanup();

    unsigned int addUIFunction(UIFunction uiFunction);
    void removeUIFunction(unsigned int id);
protected:
    UI() = default;
    ~UI();

    std::map<unsigned int, UIFunction> uiFunctions;
};

}; // namespace Echo