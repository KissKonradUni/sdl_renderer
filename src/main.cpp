#include "app.hpp"
#include "mesh.hpp"
#include "input.hpp"
#include "shader.hpp"
#include "floatmath.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

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
    // Position       / Normal
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
     0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

std::unique_ptr<Mesh> mesh      = nullptr;
std::unique_ptr<Mesh> floorMesh = nullptr;
std::unique_ptr<Shader> shader  = nullptr;

void initShaders() {
    glEnable(GL_DEPTH_TEST);

    shader    = Shader::load("assets/shaders/glsl/Basic.vert.glsl", "assets/shaders/glsl/Basic.frag.glsl");
}

void initMeshes() {
    // Put the mesh data into vectors
    std::vector<float> verticesVector(vertices, vertices + sizeof(vertices) / sizeof(float));
    std::vector<unsigned int> indicesVector(indices, indices + sizeof(indices) / sizeof(unsigned int));

    mesh      = std::make_unique<Mesh>(verticesVector, indicesVector);
    floorMesh = std::make_unique<Mesh>(verticesVector, indicesVector);

    floorMesh->position = vector4f(0.0f, -1.0f, 0.0f, 1.0f);
    floorMesh->scale    = vector4f(10.0f, 10.0f, 0.1f, 1.0f);
    floorMesh->rotation = vector4f(SDL_PI_F / 2.0f, 0.0f, 0.0f, 1.0f);
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

void initImgui() {
    SDL_Log("Initializing ImGui");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Set DPI scale to match the window
    auto displays = SDL_GetDisplays(NULL);
    auto dpi      = SDL_GetDisplayContentScale(displays[0]);
    io.FontGlobalScale = dpi;

    ImGui_ImplSDL3_InitForOpenGL(AppState->window.get(), AppState->glContext.get());
    ImGui_ImplOpenGL3_Init();

    SDL_Log("Initialized ImGui");
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    const auto result = AppState->initApp("App", "1.0", "com.sdl3.app");
    if (result != SDL_APP_CONTINUE)
        return result;
    updateWindowSize();

    initShaders();
    initMeshes();
    initEvents();
    initImgui();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
    
    return EventHandler->handle(event);
}

static double lastTime = 0.0;
SDL_AppResult SDL_AppIterate(void *appstate) {
    const double now = static_cast<double>(SDL_GetTicks()) / 1000.0;
    const double delta = now - lastTime;

    // Init ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Update camera
    camera.translation = matrix4x4f::translation(camera.position);
    camera.lookAt      = matrix4x4f::lookAt(camera.rotation);
    camera.projection  = matrix4x4f::perspective(
        80.0f * (SDL_PI_F / 180.0f), 
        viewport.w / viewport.h, 
        0.5f,   
        100.0f
    );
    
    mesh->rotation.y += delta * 0.314f;

    const auto forward = vector4f(input.x, 0.0f, input.y, 0.0f).normalize3d();
    const auto rotate  = matrix4x4f::rotation(camera.rotation.y, vector4f::up());
    const auto move    = (rotate * (forward * delta * 2.0f)).normalize3d() * 10.0f * delta;

    camera.position   = camera.position + move;
    camera.rotation.x = SDL_clamp(camera.rotation.x - input.yaw   * delta * 0.314f, -SDL_PI_F / 2.0f, SDL_PI_F / 2.0f);
    camera.rotation.y = camera.rotation.y - input.pitch * delta * 0.314f;
    if (camera.rotation.y > SDL_PI_F) camera.rotation.y -= 2.0f * SDL_PI_F;
    if (camera.rotation.y < -SDL_PI_F) camera.rotation.y += 2.0f * SDL_PI_F;
    input.yaw   = 0.0f;
    input.pitch = 0.0f;
    
    std::string title = "App - Pos: " + std::to_string(camera.position.x) + ", " + std::to_string(camera.position.y) + ", " + std::to_string(camera.position.z) + " - Rot: " + std::to_string(camera.rotation.x) + ", " + std::to_string(camera.rotation.y) + " - delta: " + std::to_string(delta);
    SDL_SetWindowTitle(AppState->window.get(), title.c_str());

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

    shader->bind();

    // TODO: Use UBO
    std::string uniformName = "camera.projection";
    shader->setUniform(uniformName, camera.projection);
    uniformName = "camera.view";
    shader->setUniform(uniformName, camera.translation * camera.lookAt);
    uniformName = "model";
    shader->setUniform(uniformName, mesh->getModelMatrix());
    mesh->draw();

    shader->setUniform(uniformName, floorMesh->getModelMatrix());
    floorMesh->draw();

    // Wait before swapping for IMGUI
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(AppState->window.get());

    lastTime = now;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_Log("Quitting with result: %d", result);

    SDL_Log("Cleaning up ImGui");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    AppState.reset();
}