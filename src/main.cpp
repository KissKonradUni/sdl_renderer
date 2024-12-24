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

void initShaders() {
    vertex   = loadShader(AppState->gpuDevice, "RawTriangle.vert", 0, 1, 0, 0);
    fragment = loadShader(AppState->gpuDevice, "SolidColor.frag" , 0, 0, 0, 0);
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

    matrix4x4f test;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    
    const auto result = AppState->initApp("App", "1.0", "com.sdl3.app");
    if (result != SDL_APP_CONTINUE)
        return result;
    updateWindowSize();

    initShaders();
    initPipeline();
    initEvents();

    runTests();

    return SDL_APP_CONTINUE;
    
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event* event) {
    return EventHandler->handle(event);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    const double now = static_cast<double>(SDL_GetTicks()) / 1000.0;
    
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
    matrix4x4f view = matrix4x4f::translation(0.0f, 0.0f, -5.0f);
    
    // Position object further back
    matrix4x4f model = matrix4x4f::translation(0.0f, 0.0f, -15.0f) *
                       matrix4x4f::rotation(now, 0.0f, 1.0f, 0.0f);
    
    float aspect = viewport.w / viewport.h;
    // Use 60 degree FOV (converted to radians)
    matrix4x4f projection = matrix4x4f::perspective(60.0f * SDL_PI_F / 180.0f, 
                                                    aspect, 
                                                    0.1f,   
                                                    100.0f);
    
    matrix4x4f matrix = projection * view * model;

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &matrix, sizeof(matrix4x4f));
    SDL_DrawGPUPrimitives(renderPass, 12, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
        SDL_Log("Couldn't submit command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}