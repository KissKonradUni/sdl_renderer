#include "app.hpp"
#include "input.hpp"
#include "shader.hpp"
#include "pipeline.hpp"
#include "floatmath.hpp"

#include <memory>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

static std::shared_ptr<SDL_GPUShader> vertex;
static std::shared_ptr<SDL_GPUShader> fragment;
static std::shared_ptr<SDL_GPUGraphicsPipeline> fillPipeline;
static SDL_GPUViewport viewport = { 0.0f, 0.0f, 1920.0f, 2000.0f, 0.0f, 1.0f };

static struct {
    vector4f position = vector4f( 0.0f,      2.0f, -5.0f,  1.0f);
    vector4f rotation = vector4f( 0.0f,  SDL_PI_F,  0.0f,  1.0f);

    matrix4x4f translation;
    matrix4x4f lookAt;
    matrix4x4f projection;
} camera;

static struct {
    float x = 0.0f;
    float y = 0.0f;
    float r = 0.0f;
} input;

void initShaders() {
    vertex   = loadShader(AppState->gpuDevice, "RawTriangle.vert", 0, 0, 0, 0);
    fragment = loadShader(AppState->gpuDevice, "SolidColor.frag" , 0, 1, 0, 0);
}

void initPipeline() {
    fillPipeline = createPipeline(vertex, fragment);
}

void updateWindowSize() {
    int w, h;
    if (SDL_GetWindowSize(AppState->window.get(), &w, &h)) {
        viewport.w = w;
        viewport.h = h;
    }
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
            case SDL_SCANCODE_Q:
                input.r = -1.0f;
                break;
            case SDL_SCANCODE_E:
                input.r = 1.0f;
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
            case SDL_SCANCODE_Q:
            case SDL_SCANCODE_E:
                input.r = 0.0f;
                break;
            default:
                break;
        }
        return SDL_APP_CONTINUE;
    });
    EventHandler->add(SDL_EVENT_WINDOW_RESIZED, [](SDL_Event* event) {
        updateWindowSize();
        return SDL_APP_CONTINUE;
    });
}

void runTests() {
    runFloatMathTests();
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    const auto result = AppState->initApp("App", "1.0", "com.sdl3.app");
    if (result != SDL_APP_CONTINUE)
        return result;
    updateWindowSize();

    initShaders();
    initPipeline();
    initEvents();

    //runTests();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    return EventHandler->handle(event);
}

static double lastTime = 0.0;
SDL_AppResult SDL_AppIterate(void *appstate) {
    const double now = static_cast<double>(SDL_GetTicks()) / 1000.0;
    const double delta = now - lastTime;
    
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(AppState->gpuDevice.get());
    if (!commandBuffer) {
        SDL_Log("Couldn't acquire command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_AcquireGPUSwapchainTexture(commandBuffer, AppState->window.get(), &swapchainTexture, NULL, NULL)) {
        SDL_Log("Couldn't get swapchain texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (swapchainTexture == NULL) {
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return SDL_APP_CONTINUE;
    }

    const auto forward = vector4f(0.0f, 0.0f, 1.0f, 0.0f) * camera.lookAt * input.y;
    const auto right = vector4f(1.0f, 0.0f, 0.0f, 0.0f) * camera.lookAt * input.x;
    const auto move = (forward + right).normalize3d() * delta * 10.0f;
    camera.position   = camera.position + move;
    camera.rotation.y = camera.rotation.y + input.r * delta * 3.14f;
    std::string title = "App - " + std::to_string(camera.position.x) + ", " + std::to_string(camera.position.y) + ", " + std::to_string(camera.position.z) + " - y rot: " + std::to_string(camera.rotation.y) + " - delta: " + std::to_string(delta);
    SDL_SetWindowTitle(AppState->window.get(), title.c_str());

    SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
    colorTargetInfo.texture     = swapchainTexture;
    colorTargetInfo.clear_color = { 
        SDL_sinf(now * 3        ) * 0.5f + 0.5f,
        SDL_sinf(now * 3 + 2.09f) * 0.5f + 0.5f,
        SDL_sinf(now * 3 + 4.18f) * 0.5f + 0.5f,
        1.0f 
    };
    colorTargetInfo.load_op     = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op    = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);
    
    SDL_SetGPUViewport(renderPass, &viewport);
    SDL_BindGPUGraphicsPipeline(renderPass, fillPipeline.get());

    // Move camera back and look at origin
    camera.translation = matrix4x4f::translation(-camera.position.x, -camera.position.y, -camera.position.z);
    camera.lookAt      = matrix4x4f::lookAt(camera.rotation);

    float aspect = viewport.h / viewport.w;
    camera.projection  = matrix4x4f::perspective(
        90.0f * SDL_PI_F / 180.0f, 
        aspect, 
        0.1f,   
        1000.0f
    );

    struct {
        matrix4x4f cameraTranslation;
        matrix4x4f cameraLookAt;
        matrix4x4f cameraProjection;
        vector4f cameraPos;
    } uniformData = { 
        camera.translation, 
        camera.lookAt, 
        camera.projection, 
        camera.position 
    };

    //SDL_PushGPUVertexUniformData(commandBuffer, 0, &uniformData, sizeof(uniformData));
    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uniformData, sizeof(uniformData));
    SDL_DrawGPUPrimitives(renderPass, 6, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
        SDL_Log("Couldn't submit command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    lastTime = now;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_Log("Quitting with result: %d", result);
}