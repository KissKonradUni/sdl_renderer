#pragma once

#include <SDL3/SDL.h>
#include <map>

typedef void (*UIFunction)();

namespace echo {

class UI {
public:
    static UI& instance() {
        static UI ui;
        return ui;
    }

    UI(const UI&) = delete;
    UI& operator=(const UI&) = delete;

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

    std::map<unsigned int, UIFunction> m_uiFunctions;
};

}; // namespace echo