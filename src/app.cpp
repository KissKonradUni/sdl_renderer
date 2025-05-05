#include "app.hpp"
#include "cinder.hpp"

#include <glad.h>

namespace cinder {

SDL_AppResult App::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        cinder::error("Couldn't initialize SDL.");
        cinder::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    cinder::log("SDL initialized.");
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
        cinder::error("Couldn't create window.");
        cinder::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    cinder::log("Window created successfully.");

    this->m_window.reset(rawWindow);
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::initGLContext() {
    SDL_GLContextState* rawGlContext = SDL_GL_CreateContext(this->m_window.get());

    if (rawGlContext == NULL) {
        cinder::error("Couldn't create GL context.");
        cinder::error(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    cinder::log("GL context created successfully.");

    cinder::log("Loading OpenGL 4.6 functions...");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        cinder::error("Failed to initialize GLAD");
        return SDL_APP_FAILURE;
    }
    gladLoadGL();
    cinder::log("OpenGL 4.6 functions loaded successfully.");

    this->m_glContext.reset(rawGlContext);
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::init(std::string appName, std::string appVersion, std::string appId) {
    SDL_SetAppMetadata(appName.c_str(), appVersion.c_str(), appId.c_str());
    
    m_console = std::make_unique<echo::Console>();
    m_console->init();
    cinder::log("Application starting...");

    SDL_AppResult result = this->initSDL();
    if (result != SDL_APP_CONTINUE)
        return result;

    result = this->initWindow();
    if (result != SDL_APP_CONTINUE)
        return result;

    result = this->initGLContext();
    if (result != SDL_APP_CONTINUE)
        return result;

    m_uiManager = std::make_unique<echo::UIManager>();
    m_uiManager->init(this->m_window.get(), this->m_glContext.get());

    m_eventManager = std::make_unique<echo::EventManager>();
    m_eventManager->init();

    m_library = std::make_unique<codex::Library>();
    m_library->init();

    cinder::log("Application initialized successfully.");

    return SDL_APP_CONTINUE;
}

void App::cleanup() {
    this->m_eventManager.reset();
    this->m_uiManager.reset();

    this->m_library.reset();
    cinder::log("Library destroyed.");
    this->m_glContext.reset();
    cinder::log("GL context destroyed.");
    this->m_window.reset();
    cinder::log("Window destroyed.");
    this->m_console.reset();
}

} // namespace cinder