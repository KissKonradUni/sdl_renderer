#pragma once

#include "console.hpp"

#include <SDL3/SDL.h>

#include <memory>
#include <string>

namespace Cinder {

class App {
public:
    static App& instance() {
        static App instance;
        return instance;
    }

    std::shared_ptr<SDL_Window>            window;
    std::shared_ptr<SDL_GLContextState> glContext;

    SDL_AppResult initSDL();
    SDL_AppResult initWindow();
    SDL_AppResult initGLContext();

    SDL_AppResult initApp(std::string appName, std::string appVersion, std::string appId);

    static SDL_Window*         getWindowPtr();
    static SDL_GLContextState* getGLContextPtr();
protected:
    App() = default;
    ~App();
};

struct CinderDeleter {
    void operator()(SDL_Window* window) {
        SDL_DestroyWindow(window);
        Echo::log("Window destroyed.");
    }
    void operator()(SDL_GLContextState* glContext) {
        SDL_GL_DestroyContext(glContext);
        Echo::log("GL context destroyed.");
    }
};

}; // namespace Cinder