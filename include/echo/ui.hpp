#pragma once

#include <SDL3/SDL.h>
#include <map>

typedef void (*UIFunction)();

namespace echo {

class UIManager {
public:
    UIManager() = default;
    ~UIManager();

    SDL_AppResult init(SDL_Window* window, SDL_GLContextState* glContext);
    void processEvent(SDL_Event* event);
    void newFrame();
    void render();
    void cleanup();

    unsigned int addUIFunction(UIFunction uiFunction);
    void removeUIFunction(unsigned int id);
protected:
    std::map<unsigned int, UIFunction> m_uiFunctions;
};

}; // namespace echo