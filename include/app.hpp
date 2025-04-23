#pragma once

#include "codex/library.hpp"
#include "echo/ui.hpp"
#include "echo/event.hpp"
#include "echo/console.hpp"

#include <SDL3/SDL.h>

#include <memory>
#include <string>

namespace cinder {

class AppDeleter {
public: 
    void operator()(SDL_Window* window) {
        SDL_DestroyWindow(window);
    }
    void operator()(SDL_GLContextState* glContext) {
        SDL_GL_DestroyContext(glContext);
    }
};

class App {
public:
    App() = default;
    ~App() = default;

    SDL_AppResult init(std::string appName, std::string appVersion, std::string appId);
    void cleanup();

    constexpr inline SDL_Window*         getWindowPtr()    const { return m_window.get(); }
    constexpr inline SDL_GLContextState* getGLContextPtr() const { return m_glContext.get(); }
    constexpr inline echo::Console*      getConsole()      const { return m_console.get(); }
    constexpr inline echo::UIManager*    getUIManager()    const { return m_uiManager.get(); }
    constexpr inline echo::EventManager* getEventManager() const { return m_eventManager.get(); }
    constexpr inline codex::Library*     getLibrary()      const { return m_library.get(); }
protected:
    SDL_AppResult initSDL();
    SDL_AppResult initWindow();
    SDL_AppResult initGLContext();

    std::unique_ptr<SDL_Window, AppDeleter>               m_window;
    std::unique_ptr<SDL_GLContextState, AppDeleter>    m_glContext;

    std::unique_ptr<echo::Console>           m_console;
    std::unique_ptr<echo::UIManager>       m_uiManager;
    std::unique_ptr<echo::EventManager> m_eventManager;

    std::unique_ptr<codex::Library>          m_library;
};

}; // namespace cinder