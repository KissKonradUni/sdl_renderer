#include "ui.hpp"
#include "app.hpp"
#include "mesh.hpp"
#include "input.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "floatmath.hpp"

#include "imgui.h"

#include <memory>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#ifdef _WIN32
    #include <windows.h>
#endif
#include <GL/gl.h>

static double lastTime = 0.0;
static double nowTime = 0.0;
static double deltaTime = 0.0;

std::unique_ptr<Mesh> mesh      = nullptr;
std::unique_ptr<Mesh> floorMesh = nullptr;
std::unique_ptr<Shader> shader  = nullptr;

std::unique_ptr<Camera> camera  = nullptr;
CameraInput cameraInput;

void initShaders() {
    camera = std::make_unique<Camera>(
        CameraViewport{0.0f, 0.0f, 1920.0f, 1200.0f},
        80.0f,
        vector4f(0.0f, 0.0f, -4.0f, 0.0f),
        vector4f::zero()
    );

    glEnable(GL_DEPTH_TEST);

    shader = Shader::load("assets/shaders/glsl/Basic.vert.glsl", "assets/shaders/glsl/Basic.frag.glsl");
}

void initMeshes() {
    mesh      = Mesh::loadMeshFromFile("assets/models/suzanne.glb");
    floorMesh = Mesh::loadMeshFromFile("assets/models/floor.glb");

    floorMesh->position.y = -1.0f;
}

void updateWindowSize() {
    if (!camera)
        return;
    
    int w, h;
    if (SDL_GetWindowSize(AppState->window.get(), &w, &h)) {
        float width  = static_cast<float>(w);
        float height = static_cast<float>(h);
        
        camera->setViewport({
            0.0f, 0.0f, width, height
        });
        camera->updateProjectionMatrix();
        
        glViewport(0, 0, width, height);
    }
}

void initEvents() {    
    EventHandler->add(SDL_EVENT_QUIT    , [](SDL_Event* event) { 
        return SDL_APP_SUCCESS; 
    });
    EventHandler->add(SDL_EVENT_KEY_DOWN, [](SDL_Event* event) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_W:
                cameraInput.movement.y = 1.0f;
                break;
            case SDL_SCANCODE_S:
                cameraInput.movement.y = -1.0f;
                break;
            case SDL_SCANCODE_A:
                cameraInput.movement.x = 1.0f;
                break;
            case SDL_SCANCODE_D:
                cameraInput.movement.x = -1.0f;
                break;
            case SDL_SCANCODE_ESCAPE:
                cameraInput.lock = !cameraInput.lock;
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
                cameraInput.movement.y = 0.0f;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_D:
                cameraInput.movement.x = 0.0f;
                break;
            default:
                break;
        }
        return SDL_APP_CONTINUE;
    });
    EventHandler->add(SDL_EVENT_MOUSE_MOTION, [](SDL_Event* event) {
        if (cameraInput.lock) return SDL_APP_CONTINUE;
        
        cameraInput.rotation.pitch += event->motion.xrel;
        cameraInput.rotation.yaw   += event->motion.yrel;

        return SDL_APP_CONTINUE;
    });
    EventHandler->add(SDL_EVENT_WINDOW_RESIZED, [](SDL_Event* event) {
        updateWindowSize();
        return SDL_APP_CONTINUE;
    });
}

void performanceWindow() {
    ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoBackground);

    ImGui::Text("Delta time: %.4f", deltaTime);
    ImGui::Text("FPS: ~%.0f", 1.0 / deltaTime);

    ImGui::End();
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    const auto result = AppState->initApp("App", "1.0", "com.sdl3.app");
    
    if (result != SDL_APP_CONTINUE)
        return result;
    
    updateWindowSize();

    initShaders();
    initMeshes();
    initEvents();

    UIManager->initUI();
    UIManager->addUIFunction(performanceWindow);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    UIManager->processEvent(event);
    
    return EventHandler->handle(event);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // ======================
    // Time management
    
    nowTime = static_cast<double>(SDL_GetTicks()) / 1000.0;
    deltaTime = nowTime - lastTime;
    lastTime = nowTime;

    // ======================
    // Process new frame

    UIManager->newFrame();
    camera->update(cameraInput, deltaTime);
    
    // ======================
    // Update "game" logic

    mesh->rotation.y += deltaTime * 0.314f;

    // ======================
    // Render

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

    shader->bind();

    // TODO: Use UBO
    std::string uniformName = "camera.projection";
    shader->setUniform(uniformName, camera->getProjectionMatrix());
    uniformName = "camera.view";
    shader->setUniform(uniformName, camera->getViewMatrix());
    uniformName = "model";
    shader->setUniform(uniformName, mesh->getModelMatrix());
    mesh->draw();

    shader->setUniform(uniformName, floorMesh->getModelMatrix());
    floorMesh->draw();

    // Draw UI on top of everything
    UIManager->render();

    // Finalize frame
    SDL_GL_SwapWindow(AppState->window.get());

    // ======================
    // Wait for next frame
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_Log("Quitting with result: %d", result);

    AppState.reset();
}