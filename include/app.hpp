#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <string>

class SDL_AppState {
public:
    std::shared_ptr<SDL_Window>            window;
    std::shared_ptr<SDL_GLContextState> glContext;

    SDL_AppResult initSDL();
    SDL_AppResult initWindow();
    SDL_AppResult initGLContext();

    SDL_AppResult initApp(std::string appName, std::string appVersion, std::string appId);
};

class SDL_AppDeleter {
public:
    void operator()(SDL_Window* window) {
        SDL_DestroyWindow(window);
        SDL_Log("Window destroyed");
    }
    void operator()(SDL_GLContextState* glContext) {
        SDL_GL_DestroyContext(glContext);
        SDL_Log("GL context destroyed");
    }
    void operator()(SDL_AppState* appState) {
        SDL_Log("Window released from GPU device");

        appState->window.reset();
        appState->glContext.reset();
    }
};
extern SDL_AppDeleter AppDeleter;

extern std::unique_ptr<SDL_AppState, SDL_AppDeleter> AppState;