#pragma once

#include "echo/console.hpp"

#include <SDL3/SDL.h>

#include <memory>
#include <string>

namespace cinder {

class App {
public:
    static App& instance() {
        static App instance;
        return instance;
    }

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    std::shared_ptr<SDL_Window>            m_window;
    std::shared_ptr<SDL_GLContextState> m_glContext;

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

class CinderDeleter {
public: 
    void operator()(SDL_Window* window) {
        SDL_DestroyWindow(window);
        echo::log("Window destroyed.");
    }
    void operator()(SDL_GLContextState* glContext) {
        SDL_GL_DestroyContext(glContext);
        echo::log("GL context destroyed.");
    }
};

}; // namespace cinder