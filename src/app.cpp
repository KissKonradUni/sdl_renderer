#include "app.hpp"

#include <glad.h>

namespace cinder {

CinderDeleter cinderDeleter;

SDL_AppResult App::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        echo::error("Couldn't initialize SDL.");
        echo::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    echo::log("SDL initialized.");
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
        echo::error("Couldn't create window.");
        echo::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    echo::log("Window created successfully.");

    this->m_window.reset(rawWindow, cinderDeleter);
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::initGLContext() {
    SDL_GLContextState* rawGlContext = SDL_GL_CreateContext(this->m_window.get());

    if (rawGlContext == NULL) {
        echo::error("Couldn't create GL context.");
        echo::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    echo::log("GL context created successfully.");

    echo::log("Loading OpenGL 4.6 functions...");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        echo::error("Failed to initialize GLAD");
        return SDL_APP_FAILURE;
    }
    gladLoadGL();
    echo::log("OpenGL 4.6 functions loaded successfully.");

    this->m_glContext.reset(rawGlContext, cinderDeleter);
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::initApp(std::string appName, std::string appVersion, std::string appId) {
    SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appId.c_str());
    echo::log("Application starting...");

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
    this->m_glContext.reset();
    this->m_window.reset();
}

} // namespace cinder