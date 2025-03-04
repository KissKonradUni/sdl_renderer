#include "app.hpp"

#include "lib/glad/glad.h"

namespace Cinder {

CinderDeleter cinderDeleter;

SDL_AppResult App::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        Echo::error("Couldn't initialize SDL.");
        Echo::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    Echo::log("SDL initialized.");
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::initWindow() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    // TODO: Set the OpenGL profile to core when shaders are implemented
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

    SDL_Window* rawWindow = SDL_CreateWindow("Cinder", 1920, 1200, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (rawWindow == NULL) {
        Echo::error("Couldn't create window.");
        Echo::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    Echo::log("Window created successfully.");

    this->m_window.reset(rawWindow, cinderDeleter);
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::initGLContext() {
    SDL_GLContextState* rawGlContext = SDL_GL_CreateContext(this->m_window.get());

    if (rawGlContext == NULL) {
        Echo::error("Couldn't create GL context.");
        Echo::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    Echo::log("GL context created successfully.");

    Echo::log("Loading OpenGL 4.6 functions...");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        Echo::error("Failed to initialize GLAD");
        return SDL_APP_FAILURE;
    }
    gladLoadGL();
    Echo::log("OpenGL 4.6 functions loaded successfully.");

    this->m_glContext.reset(rawGlContext, cinderDeleter);
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::initApp(std::string appName, std::string appVersion, std::string appId) {
    SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appId.c_str());
    Echo::log("Application starting...");

    SDL_AppResult result = this->initSDL();
    if (result != SDL_APP_CONTINUE)
        return result;

    result = this->initWindow();
    if (result != SDL_APP_CONTINUE)
        return result;

    result = this->initGLContext();
    if (result != SDL_APP_CONTINUE)
        return result;

    return SDL_APP_CONTINUE;
}

SDL_Window* App::getWindowPtr() {
    return App::instance().m_window.get();
}

SDL_GLContextState* App::getGLContextPtr() {
    return App::instance().m_glContext.get();
}

App::~App() {
    Echo::log("Window released from GPU device.");

    this->m_window.reset();
    this->m_glContext.reset();
}

} // namespace Cinder