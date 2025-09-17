#include "app.hpp"
#include "cinder.hpp"
#include "floatmath.hpp"

#include "hex/components/rendererComponent.hpp"
#include "hex/components/transformComponent.hpp"
#include "hex/scene.hpp"
#include "hex/actor.hpp"
#include "hex/camera.hpp"
#include "hex/framebuffer.hpp"
#include "hex/lighting.hpp"

#include "echo/ui.hpp"
#include "echo/event.hpp"
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

namespace cinder {

using namespace codex;
using namespace hex;
using namespace echo;

// TODO: time struct in app
double lastTime   = 0.0;
double nowTime    = 0.0;
double deltaTime  = 0.0;

std::unique_ptr<cinder::App> app;

// TODO: make some kind of manager, move to app
Scene scene;

// TODO: make a component for this, add to actor, move to scene
std::unique_ptr<Camera> camera = nullptr;
std::unique_ptr<UniformBuffer> cameraUniformBuffer = nullptr;
// TODO: make it a part of input maybe?
CameraInput cameraInput;

// TODO: make it part of prism (rendering module)
typedef struct { int x, y; float dpi; } windowStruct;
windowStruct lastFrameWindowSize{100, 100, 1.0f};
std::unique_ptr<GBuffer> sceneFramebuffer = nullptr;
std::unique_ptr<Framebuffer> combinedFramebuffer = nullptr;
std::unique_ptr<Framebuffer> radianceProbesFramebuffer = nullptr;

codex::Mesh *quadMesh = nullptr;
codex::Shader *combineShader = nullptr;
codex::Shader *probesShader = nullptr;

void initCamera() {
    camera = std::make_unique<Camera>(
        CameraViewport{0.0f, 0.0f, 1920.0f, 1200.0f},
        80.0f,
        vector4f(0.0f, 0.0f, 4.0f, 0.0f),
        vector4f::zero()
    );
    cameraUniformBuffer = std::make_unique<UniformBuffer>(sizeof(CameraUniformBufferData), 0);
}

void initGLParams() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void initDebugStuff() {
    using namespace cinder;

    auto library = app->getLibrary();

    std::filesystem::path assetPath = "./assets/shaders/glsl/Deferred.shader";
    library->formatPath(&assetPath);
    auto shaderNode = library->tryGetAssetNode(assetPath);
    auto shader = library->tryLoadResource<Shader>(shaderNode);

    assetPath = "./assets/materials/Plaster.material";
    library->formatPath(&assetPath);
    auto materialNode = library->tryGetAssetNode(assetPath);
    auto material = library->tryLoadResource<Material>(materialNode);

    assetPath = "./assets/models/quad.glb";
    library->formatPath(&assetPath);
    auto quadNode = library->tryGetAssetNode(assetPath);
    quadMesh = library->tryLoadResource<Mesh>(quadNode);

    assetPath = "./assets/shaders/glsl/PBRBasePass.shader";
    library->formatPath(&assetPath);
    auto combineNode = library->tryGetAssetNode(assetPath);
    combineShader = library->tryLoadResource<Shader>(combineNode);

    assetPath = "./assets/shaders/glsl/Probes.shader";
    library->formatPath(&assetPath);
    auto probesNode = library->tryGetAssetNode(assetPath);
    probesShader = library->tryLoadResource<Shader>(probesNode);

    // Load later so shaders are ready
    assetPath = "./assets/models/sponza.glb";
    //assetPath = "./assets/models/NewSponza_Main_glTF_003.gltf";
    library->formatPath(&assetPath);
    auto meshNode = library->tryGetAssetNode(assetPath);
    auto mesh = library->tryLoadResource<Mesh>(meshNode);

    Actor* actor = scene.newActor();
    actor->setName("Sponza");
    actor->addComponent<TransformComponent>();
    actor->addComponent<RendererComponent>(shader, material, mesh);

    /*
    assetPath = "./assets/models/curtains.glb";
    library->formatPath(&assetPath);
    auto meshNode2 = library->tryGetAssetNode(assetPath);
    auto mesh2 = library->tryLoadResource<Mesh>(meshNode2);

    actor = scene.newActor();
    actor->setName("Curtains");
    actor->addComponent<TransformComponent>();
    actor->addComponent<RendererComponent>(shader, material, mesh2);
    */
}

void initEvents() {    
    auto events = app->getEventManager();

    events->add(SDL_EVENT_QUIT    , [](SDL_Event* event) { 
        return SDL_APP_SUCCESS; 
    });
    events->add(SDL_EVENT_KEY_DOWN, [](SDL_Event* event) {
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
    events->add(SDL_EVENT_KEY_UP, [](SDL_Event* event) {
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
    events->add(SDL_EVENT_MOUSE_MOTION, [](SDL_Event* event) {        
        cameraInput.rotation.pitch += event->motion.xrel;
        cameraInput.rotation.yaw   += event->motion.yrel;

        return SDL_APP_CONTINUE;
    });
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

unsigned int targetHandle = -1;

void renderWindow() {
    ImGui::Begin("Render", nullptr);

    int startPos = ImGui::GetCursorPos().y;

    const auto dpi = SDL_GetWindowPixelDensity(app->getWindowPtr());
    int width  = ImGui::GetContentRegionAvail().x * dpi;
    int height = ImGui::GetContentRegionAvail().y * dpi;

    if (lastFrameWindowSize.x != width || lastFrameWindowSize.y != height) {
        lastFrameWindowSize.x = width;
        lastFrameWindowSize.y = height;
        lastFrameWindowSize.dpi = dpi;

        camera->setViewport({
            0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)
        });
        camera->updateProjectionMatrix();
        glViewport(0, 0, width, height);

        sceneFramebuffer->resize(width, height);
        combinedFramebuffer->resize(width, height);
    }

    if (targetHandle == -1) {
        targetHandle = combinedFramebuffer->getColorTarget().getHandle();
    }

    ImGui::Image(
        targetHandle,
        ImVec2(width / dpi, height / dpi),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    ImGui::SetCursorPos(ImVec2(10, startPos));
    ImGui::Text("Input: %s", cameraInput.lock ? "Locked" : "Captured");

    ImGui::End();
}

void debugWindow() {
    ImGui::Begin("Debug", nullptr);

    ImGui::Text("Current render target: %u", targetHandle);
    
    unsigned int combinedHandle = combinedFramebuffer->getColorTarget().getHandle();
    bool combinedTarget = ImGui::RadioButton("Combined", targetHandle == combinedHandle);
    if (combinedTarget) targetHandle = combinedHandle;

    unsigned int colorHandle = sceneFramebuffer->getColorTarget().getHandle();
    bool colorTarget = ImGui::RadioButton("Color", targetHandle == colorHandle);
    if (colorTarget) targetHandle = colorHandle;

    unsigned int normalHandle = sceneFramebuffer->getNormalTarget().getHandle();
    bool normalTarget = ImGui::RadioButton("Normal", targetHandle == normalHandle);
    if (normalTarget) targetHandle = normalHandle;

    unsigned int positionHandle = sceneFramebuffer->getPositionTarget().getHandle();
    bool positionTarget = ImGui::RadioButton("Position", targetHandle == positionHandle);
    if (positionTarget) targetHandle = positionHandle;

    unsigned int aoRoughnessMetallicHandle = sceneFramebuffer->getAORoughnessMetallicTarget().getHandle();
    bool aoRoughnessMetallicTarget = ImGui::RadioButton("AO/Roughness/Metallic", targetHandle == aoRoughnessMetallicHandle);
    if (aoRoughnessMetallicTarget) targetHandle = aoRoughnessMetallicHandle;

    unsigned int radianceProbesHandle = radianceProbesFramebuffer->getColorTarget().getHandle();
    bool radianceProbesTarget = ImGui::RadioButton("Radiance Probes", targetHandle == radianceProbesHandle);
    if (radianceProbesTarget) targetHandle = radianceProbesHandle;

    ImGui::End();
}

} // namespace cinder

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    using namespace cinder;

    app.reset(new App());
    SDL_AppResult result = app->init("Cinder", "1.0", "hu.konrads.cinder");

    if (result != SDL_APP_CONTINUE)
        return result;

    initCamera();
    initGLParams();
    initEvents();

    initDebugStuff();

    sceneFramebuffer = std::make_unique<GBuffer>(1920, 1200);
    combinedFramebuffer = std::make_unique<Framebuffer>(1920, 1200);
    radianceProbesFramebuffer = std::make_unique<Framebuffer>(RADIANCE_TEXTURE_SIZE, RADIANCE_TEXTURE_SIZE);

    // Enable adaptive vsync
    SDL_GL_SetSwapInterval(-1);

    auto ui = app->getUIManager();
    ui->addUIFunction(renderWindow);
    ui->addUIFunction(performanceWindow);
    ui->addUIFunction(debugWindow);
    ui->addUIFunction(Library::assetsWindow);
    ui->addUIFunction([]() {
        app->getConsole()->drawConsole();
        camera->cameraWindow();
        scene.editorUI();
    });

    //ui->openAssetBrowserDialog(0, [](FileNode* node) {
    //    // Test
    //});

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    using namespace cinder;

           app->getUIManager()->processEvent(event);
    return app->getEventManager()->handle(event);
}

#define NS_TO_S 0.000'000'001

SDL_AppResult SDL_AppIterate(void *appstate) {
    using namespace cinder;

    // ======================
    // Upadte asset library
    app->getLibrary()->checkForFinishedAsync();

    // ======================
    // Time management
    
    nowTime = static_cast<double>(SDL_GetTicksNS() * NS_TO_S);
    deltaTime = nowTime - lastTime;
    lastTime = nowTime;

    // ======================
    // Process new frame

    app->getUIManager()->newFrame();
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

        glViewport(0, 0, lastFrameWindowSize.x, lastFrameWindowSize.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        cameraUniformBuffer->updateData(camera->getShaderBufferPointer());
        scene.render();

    sceneFramebuffer->unbind();

    if (combineShader->isInitialized() && quadMesh->isInitialized()) {

    combinedFramebuffer->bind();

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

        glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFramebuffer->getHandle());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, combinedFramebuffer->getHandle());
        glBlitFramebuffer(
            0, 0, lastFrameWindowSize.x, lastFrameWindowSize.y,
            0, 0, lastFrameWindowSize.x, lastFrameWindowSize.y,
            GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );
        glBindFramebuffer(GL_FRAMEBUFFER, combinedFramebuffer->getHandle());

        combineShader->bind();
        combineShader->setUniform("gDiffuse", 0);
        combineShader->setUniform("gNormal", 1);
        combineShader->setUniform("gPosition", 2);
        combineShader->setUniform("gAORoughnessMetallic", 3);
        combineShader->setUniform("screenSize", vector4f(
            static_cast<float>(lastFrameWindowSize.x),
            static_cast<float>(lastFrameWindowSize.y),
            0.0f,
            0.0f
        ));

        sceneFramebuffer->getColorTarget().bind(0);
        sceneFramebuffer->getNormalTarget().bind(1);
        sceneFramebuffer->getPositionTarget().bind(2);
        sceneFramebuffer->getAORoughnessMetallicTarget().bind(3);

        glDisable(GL_CULL_FACE);
        cameraUniformBuffer->updateData(camera->getShaderBufferPointer());
        quadMesh->draw();

    combinedFramebuffer->unbind();

    }

    if (probesShader->isInitialized()) {

    radianceProbesFramebuffer->bind();

        glViewport(0, 0, RADIANCE_TEXTURE_SIZE, RADIANCE_TEXTURE_SIZE);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        probesShader->bind();
        /*
        probesShader->setUniform("gDiffuse", 0);
        probesShader->setUniform("gNormal", 1);
        probesShader->setUniform("gPosition", 2);
        probesShader->setUniform("gAORoughnessMetallic", 3);
        probesShader->setUniform("screenAndTextureSize", vector4f(
            static_cast<float>(lastFrameWindowSize.x),
            static_cast<float>(lastFrameWindowSize.y),
            static_cast<float>(RADIANCE_TEXTURE_SIZE),
            static_cast<float>(RADIANCE_TEXTURE_SIZE)
        ));
        */

        sceneFramebuffer->getColorTarget().bind(0);
        sceneFramebuffer->getNormalTarget().bind(1);
        sceneFramebuffer->getPositionTarget().bind(2);
        sceneFramebuffer->getAORoughnessMetallicTarget().bind(3);
        
        glDisable(GL_CULL_FACE);
        cameraUniformBuffer->updateData(camera->getShaderBufferPointer());
        quadMesh->draw();

    radianceProbesFramebuffer->unbind();

    }

    // Draw UI on top of everything
    app->getUIManager()->render();

    // Finalize frame
    SDL_GL_SwapWindow(app->getWindowPtr());

    // ======================
    // Wait for next frame
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {    
    using namespace cinder;
    
    log(std::format("Application quit with result: {}", (uint8_t)result));
    app->cleanup();
}