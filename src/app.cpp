#include "app.hpp"

#include "glad/glad.h"

SDL_AppDeleter AppDeleter;

std::unique_ptr<SDL_AppState, SDL_AppDeleter> AppState(new SDL_AppState(), AppDeleter);

SDL_AppResult SDL_AppState::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Log("SDL initialized");
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppState::initWindow() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    // TODO: Set the OpenGL profile to core when shaders are implemented
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

    SDL_Window* rawWindow = SDL_CreateWindow("App", 1920, 1200, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (rawWindow == NULL) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Log("Window created");

    AppState->window.reset(rawWindow, AppDeleter);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppState::initGLContext() {
    SDL_GLContextState* rawGlContext = SDL_GL_CreateContext(this->window.get());

    if (rawGlContext == NULL) {
        SDL_Log("Couldn't create GL context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("Loading OpenGL 4.6 functions...");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to initialize GLAD");
        return SDL_APP_FAILURE;
    }
    gladLoadGL();
    SDL_Log("Context created successfully, OpenGL 4.6 functions loaded");

    AppState->glContext.reset(rawGlContext, AppDeleter);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppState::initApp(std::string appName, std::string appVersion, std::string appId) {
    SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appId.c_str());
    SDL_Log("\nApplication starting...");

    SDL_AppResult result = AppState->initSDL();
    if (result != SDL_APP_CONTINUE)
        return result;

    result = AppState->initWindow();
    if (result != SDL_APP_CONTINUE)
        return result;

    result = AppState->initGLContext();
    if (result != SDL_APP_CONTINUE)
        return result;

    return SDL_APP_CONTINUE;
}