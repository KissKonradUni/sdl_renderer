#include "app.hpp"
#include "floatmath.hpp"

#include "hex/scene.hpp"
#include "hex/actor.hpp"
#include "hex/camera.hpp"
#include "hex/framebuffer.hpp"
#include "hex/components/transformComponent.hpp"

#include "echo/ui.hpp"
#include "echo/input.hpp"
#include "echo/console.hpp"

#include "codex/mesh.hpp"
#include "codex/shader.hpp"
#include "codex/library.hpp"

#include "imgui.h"

#include <memory>
#include <array>
#include <stdlib.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#ifdef _WIN32
    #include <windows.h>
#endif
#include <GL/gl.h>

std::unique_ptr<Codex::UniformBuffer> cameraBuffer = nullptr;

std::unique_ptr<Hex::Camera> camera = nullptr;
Hex::CameraInput cameraInput{{0.0f, 0.0f}, {0.0f, 0.0f}, true};

struct { int x, y; bool changed; } lastFrameWindowSize {100, 100, false};
std::unique_ptr<Hex::Framebuffer> sceneFramebuffer = nullptr;

Codex::Mesh* mesh = nullptr;
transformf meshTransform;

Hex::Scene scene;

void performanceWindow();
void renderWindow();

void initCamera() {
    camera = std::make_unique<Hex::Camera>(
        Hex::CameraViewport{0.0f, 0.0f, 1920.0f, 1200.0f},
        80.0f,
        vector4f(0.0f, 0.0f, 4.0f, 0.0f),
        vector4f::zero()
    );
    cameraBuffer = std::make_unique<Codex::UniformBuffer>(sizeof(Hex::CameraUniformBufferData), 0);
}

void initGLParams() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void initDebugStuff() {
    std::filesystem::path assetPath = "./assets/shaders/glsl/Basic.shader";
    Codex::Library::instance().formatPath(&assetPath);
    auto shaderNode = Codex::Library::instance().tryGetAssetNode(assetPath);
    shader = Codex::Library::instance().tryLoadResource<Codex::Shader>(shaderNode);

    assetPath = "./assets/models/sponza.glb";
    Codex::Library::instance().formatPath(&assetPath);
    auto meshNode = Codex::Library::instance().tryGetAssetNode(assetPath);
    mesh = Codex::Library::instance().tryLoadResource<Codex::Mesh>(meshNode);

    Hex::Actor* actor = new Hex::Actor();
    actor->addComponent<Hex::TransformComponent>();
    scene.addActor(actor);

    Hex::Actor* actor2 = new Hex::Actor(actor);
    actor2->addComponent<Hex::TransformComponent>();
    scene.addActor(actor2);

    Hex::Actor* actor3 = new Hex::Actor(actor2);
    actor3->addComponent<Hex::TransformComponent>();
    scene.addActor(actor3);

    actor = new Hex::Actor();
    actor->addComponent<Hex::TransformComponent>();
    scene.addActor(actor);

    actor2 = new Hex::Actor(actor);
    actor2->addComponent<Hex::TransformComponent>();
    scene.addActor(actor2);
}

void initEvents() {    
    Echo::Events::add(SDL_EVENT_QUIT    , [](SDL_Event* event) { 
        return SDL_APP_SUCCESS; 
    });
    Echo::Events::add(SDL_EVENT_KEY_DOWN, [](SDL_Event* event) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_W:
                cameraInput.movement.y = -1.0f;
                break;
            case SDL_SCANCODE_S:
                cameraInput.movement.y = 1.0f;
                break;
            case SDL_SCANCODE_A:
                cameraInput.movement.x = -1.0f;
                break;
            case SDL_SCANCODE_D:
                cameraInput.movement.x = 1.0f;
                break;
            case SDL_SCANCODE_SPACE:
                cameraInput.movement.z = 1.0f;
                break;
            case SDL_SCANCODE_LSHIFT:
                cameraInput.movement.z = -1.0f;
                break;
            case SDL_SCANCODE_ESCAPE:
                cameraInput.lock = !cameraInput.lock;
                cameraInput.rotation = {0.0f, 0.0f};
                break;
            default:
                break;
        }
        return SDL_APP_CONTINUE;
    });
    Echo::Events::add(SDL_EVENT_KEY_UP, [](SDL_Event* event) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_S:
                cameraInput.movement.y = 0.0f;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_D:
                cameraInput.movement.x = 0.0f;
                break;
            case SDL_SCANCODE_SPACE:
            case SDL_SCANCODE_LSHIFT:
                cameraInput.movement.z = 0.0f;
                break;
            default:
                break;
        }
        return SDL_APP_CONTINUE;
    });
    Echo::Events::add(SDL_EVENT_MOUSE_MOTION, [](SDL_Event* event) {        
        cameraInput.rotation.pitch += event->motion.xrel;
        cameraInput.rotation.yaw   += event->motion.yrel;

        return SDL_APP_CONTINUE;
    });
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    const auto result = Cinder::App::instance().initApp("Cinder", "1.0", "hu.konrads.cinder");

    if (result != SDL_APP_CONTINUE)
        return result;

    initCamera();
    initGLParams();
    initEvents();
    Codex::Library::instance().init();

    initDebugStuff();

    sceneFramebuffer = std::make_unique<Hex::Framebuffer>(1920, 1200);

    // Enable adaptive vsync
    SDL_GL_SetSwapInterval(-1);

    Echo::UI::instance().initUI();
    Echo::UI::instance().addUIFunction(renderWindow);
    Echo::UI::instance().addUIFunction(performanceWindow);
    Echo::UI::instance().addUIFunction(Echo::consoleWindow);
    Echo::UI::instance().addUIFunction(Codex::Library::assetsWindow);
    Echo::UI::instance().addUIFunction([]() {
        camera->cameraWindow();
        scene.editorUI();
    });

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    Echo::UI::instance().processEvent(event);
    
    return Echo::Events::handle(event);
}

static double lastTime   = 0.0;
static double nowTime    = 0.0;
static double deltaTime  = 0.0;

SDL_AppResult SDL_AppIterate(void *appstate) {
    // ======================
    // Upadte asset library
    Codex::Library::instance().checkForFinishedAsync();

    // ======================
    // Time management
    
    nowTime = static_cast<double>(SDL_GetTicks()) / 1000.0;
    deltaTime = nowTime - lastTime;
    lastTime = nowTime;

    // ======================
    // Process new frame

    Echo::UI::instance().newFrame();
    camera->update(cameraInput, deltaTime);
    
    // ======================
    // Update "game" logic

    // if (mesh)
    //     mesh->rotation.y += deltaTime * 0.314f;

    // ======================
    // Render

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sceneFramebuffer->bind();

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

        cameraBuffer->updateData(camera->getShaderBufferPointer());
        //Codex::Assets::instance().getCurrentScene().draw();

        /*
        if (shader && material && mesh) {
            shader->getShader()->bind();
            material->bindTextures();
            cameraBuffer->updateData(camera->getShaderBufferPointer());
            shader->getShader()->setUniform("modelMatrix", mesh->transform.getModelMatrix());
            shader->getShader()->setUniform("textureDiffuse", 0);
            shader->getShader()->setUniform("textureNormal", 1);
            shader->getShader()->setUniform("textureAORoughnessMetallic", 2);
            //packedMesh->draw();
            mesh->draw();
        }
        */

        if (shader->isInitialized()) {
            shader->bind();
            shader->setUniform("modelMatrix", meshTransform.getModelMatrix());
        }

        if (mesh->isInitialized()) {
            mesh->draw();
        }

    sceneFramebuffer->unbind();

    // Draw UI on top of everything
    Echo::UI::instance().render();

    // Finalize frame
    SDL_GL_SwapWindow(Cinder::App::getWindowPtr());

    // ======================
    // Wait for next frame
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    Echo::log(std::string("Application quit with result: ") + std::to_string(result));
}

size_t getCurrentMemoryUsage() {
    // If not on Linux, return 0
    // TODO: Implement for other platforms
    #ifndef __linux__
        return 0;
    #endif

    size_t memUsage = 0;
    FILE* file = fopen("/proc/self/status", "r");
    if (file) {
        char line[128];
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                // Extract the resident memory size in KB
                sscanf(line, "VmRSS: %zu", &memUsage);
                break;
            }
        }
        fclose(file);
    }
    return memUsage; // Returns memory usage in KB
}

std::array<float, 256> frameTimes = {};
std::array<float, 10> frameTimesAvg = {};

int frameTimeAvgIndex = 0, frameTimeIndex = 0;

std::array<float, 256> memoryUsage = {};
int memoryUsageIndex = 0;

void performanceWindow() {
    ImGui::Begin("Performance", nullptr);

    ImGui::Text("Delta time: ~%0.04f ", deltaTime);
    float fps = 1.0f / deltaTime;
    ImGui::Text("FPS: ~%03.00f", fps);

    frameTimesAvg[frameTimeAvgIndex] = fps;
    frameTimeAvgIndex = (frameTimeAvgIndex + 1) % frameTimesAvg.size();

    if (frameTimeAvgIndex == 0) {
        float sum = 0.0f;
        for (const auto& time : frameTimesAvg) {
            sum += time;
        }
        frameTimes[frameTimeIndex] = sum / frameTimesAvg.size();
        frameTimeIndex = (frameTimeIndex + 1) % frameTimes.size();
    }

    ImGui::Separator();

    float windowWidth = ImGui::GetWindowWidth();

    ImGui::BeginChild("AvgFpsGraph", ImVec2(windowWidth / 2, -1));
        ImGui::PlotLines("##fps", frameTimes.data(), frameTimes.size(), frameTimeIndex,
                         "Avg frames per second", 30.0f, 200.0f,
                         ImVec2(windowWidth / 2, -1));
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("PerfGraph", ImVec2(windowWidth / 2, -1));
        memoryUsage[memoryUsageIndex] = static_cast<float>(getCurrentMemoryUsage()) / 1024.0f; // Convert to MB
        memoryUsageIndex = (memoryUsageIndex + 1) % memoryUsage.size();
        
        ImGui::PlotHistogram("##memgraph", memoryUsage.data(), memoryUsage.size(), memoryUsageIndex, 
                             "Memory usage (MB)", 0, 512.0f,
                             ImVec2(windowWidth / 2, -1));
    ImGui::EndChild();

    ImGui::End();
}

void renderWindow() {
    ImGui::Begin("Render", nullptr);

    int startPos = ImGui::GetCursorPos().y;

    int width  = ImGui::GetContentRegionAvail().x;
    int height = ImGui::GetContentRegionAvail().y;

    if (lastFrameWindowSize.x != width || lastFrameWindowSize.y != height) {
        lastFrameWindowSize.x = width;
        lastFrameWindowSize.y = height;
        lastFrameWindowSize.changed = true;

        camera->setViewport({
            0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)
        });
        camera->updateProjectionMatrix();
        glViewport(0, 0, width, height);

        sceneFramebuffer->resize(width, height);
    }

    ImGui::Image(
        sceneFramebuffer->getColorTarget().getHandle(),
        ImVec2(width, height),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    ImGui::SetCursorPos(ImVec2(0, startPos));
    ImGui::Text("Input: %s", cameraInput.lock ? "Locked" : "Captured");

    ImGui::End();
}