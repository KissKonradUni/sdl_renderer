#include "app.hpp"
#include "mesh.hpp"
#include "input.hpp"
#include "shader.hpp"
#include "floatmath.hpp"

#include <memory>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#ifdef _WIN32
    #include <windows.h>
#endif
#include <GL/gl.h>

static SDL_GPUViewport viewport = { 0.0f, 0.0f, 1920.0f, 2000.0f, 0.0f, 1.0f };

static struct {
    vector4f position = vector4f( 0.0f,  0.0f, -3.0f,  1.0f);
    vector4f rotation = vector4f( 0.0f,  0.0f,  0.0f,  1.0f);

    matrix4x4f translation;
    matrix4x4f lookAt;
    matrix4x4f projection;
} camera;

static struct {
    float x = 0.0f;
    float y = 0.0f;

    float pitch = 0.0f;
    float yaw   = 0.0f;

    bool lock = true;
} input;

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

std::unique_ptr<Mesh> mesh = nullptr;
std::unique_ptr<Shader> shader = nullptr;

void initShaders() {
    // Put the mesh data into vectors
    std::vector<float> verticesVector(vertices, vertices + sizeof(vertices) / sizeof(float));
    std::vector<unsigned int> indicesVector(indices, indices + sizeof(indices) / sizeof(unsigned int));

    mesh   = std::make_unique<Mesh>(verticesVector, indicesVector);
    shader = Shader::load("assets/shaders/glsl/Basic.vert.glsl", "assets/shaders/glsl/Basic.frag.glsl");
}

void updateWindowSize() {
    int w, h;
    if (SDL_GetWindowSize(AppState->window.get(), &w, &h)) {
        viewport.w = w;
        viewport.h = h;
    }
    glViewport(0, 0, viewport.w, viewport.h);
}

void initEvents() {    
    EventHandler->add(SDL_EVENT_QUIT    , [](SDL_Event* event) { 
        return SDL_APP_SUCCESS; 
    });
    EventHandler->add(SDL_EVENT_KEY_DOWN, [](SDL_Event* event) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_ESCAPE:
                return SDL_APP_SUCCESS;
            case SDL_SCANCODE_W:
                input.y = 1.0f;
                break;
            case SDL_SCANCODE_S:
                input.y = -1.0f;
                break;
            case SDL_SCANCODE_A:
                input.x = 1.0f;
                break;
            case SDL_SCANCODE_D:
                input.x = -1.0f;
                break;
            case SDL_SCANCODE_INSERT:
                input.lock = !input.lock;
                break;
            default:
                break;
        }
        return SDL_APP_CONTINUE;
    });
    EventHandler->add(SDL_EVENT_KEY_UP, [](SDL_Event* event) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_S:
                input.y = 0.0f;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_D:
                input.x = 0.0f;
                break;
            default:
                break;
        }
        return SDL_APP_CONTINUE;
    });
    EventHandler->add(SDL_EVENT_MOUSE_MOTION, [](SDL_Event* event) {
        if (input.lock) return SDL_APP_CONTINUE;
        
        input.pitch += event->motion.xrel;
        input.yaw   += event->motion.yrel;

        return SDL_APP_CONTINUE;
    });
    EventHandler->add(SDL_EVENT_WINDOW_RESIZED, [](SDL_Event* event) {
        updateWindowSize();
        return SDL_APP_CONTINUE;
    });
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    const auto result = AppState->initApp("App", "1.0", "com.sdl3.app");
    if (result != SDL_APP_CONTINUE)
        return result;
    updateWindowSize();

    initShaders();
    initEvents();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    return EventHandler->handle(event);
}

static double lastTime = 0.0;
SDL_AppResult SDL_AppIterate(void *appstate) {
    const double now = static_cast<double>(SDL_GetTicks()) / 1000.0;
    const double delta = now - lastTime;
    
    const auto forward = vector4f::front() * camera.lookAt * input.y;
    const auto right   = vector4f::right() * camera.lookAt * input.x;
    const auto move    = (forward + right).normalize3d() * delta * 10.0f;

    camera.position   = camera.position + move;
    camera.rotation.x = SDL_clamp(camera.rotation.x - input.yaw   * delta * 0.314f, -SDL_PI_F / 2.0f, SDL_PI_F / 2.0f);
    camera.rotation.y = SDL_clamp(camera.rotation.y - input.pitch * delta * 0.314f, -SDL_PI_F / 2.0f, SDL_PI_F / 2.0f);
    input.yaw   = 0.0f;
    input.pitch = 0.0f;
    
    std::string title = "App - " + std::to_string(camera.position.x) + ", " + std::to_string(camera.position.y) + ", " + std::to_string(camera.position.z) + " - y rot: " + std::to_string(camera.rotation.y) + " - delta: " + std::to_string(delta);
    SDL_SetWindowTitle(AppState->window.get(), title.c_str());

    // Move camera back and look at origin
    camera.translation = matrix4x4f::translation(camera.position);
    camera.lookAt      = matrix4x4f::lookAt(camera.rotation);

    float aspect = viewport.w / viewport.h;
    camera.projection  = matrix4x4f::perspective(
        80.0f * (SDL_PI_F / 180.0f), 
        aspect, 
        0.5f,   
        100.0f
    );

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

    shader->bind();

    std::string uniformName = "projection";
    shader->setUniform(uniformName, camera.projection);
    uniformName = "view";
    shader->setUniform(uniformName, camera.translation * camera.lookAt);
    uniformName = "model";
    shader->setUniform(uniformName, matrix4x4f::identity());

    mesh->draw();

    SDL_GL_SwapWindow(AppState->window.get());

    lastTime = now;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_Log("Quitting with result: %d", result);

    AppState.reset();
}