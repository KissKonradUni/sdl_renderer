#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_video.h"
#include "app.hpp"
#include "shader.hpp"
#include "pipeline.hpp"

#include <memory>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

static std::shared_ptr<SDL_GPUShader> vertex;
static std::shared_ptr<SDL_GPUShader> fragment;
static std::shared_ptr<SDL_GPUGraphicsPipeline> fillPipeline;
static SDL_GPUViewport viewport = { 0.0f, 0.0f, 2000.0f, 2000.0f, 0.0f, 1.0f };

void initShaders() {
    vertex   = LoadShader(AppState->gpuDevice, "RawTriangle.vert", 0, 0, 0, 0);
    fragment = LoadShader(AppState->gpuDevice, "SolidColor.frag" , 0, 0, 0, 0);
}

void initPipeline() {
    fillPipeline = CreatePipeline(vertex, fragment);
}

void updateWindowSize() {
    int w, h;
    if (SDL_GetWindowSize(AppState->window.get(), &w, &h)) {
        viewport.w = w;
        viewport.h = h;
    }
    
    if (w > h) {
        viewport.x = (w - h) / 2.0f;
        viewport.y = 0;
        viewport.w = h;
    } else {
        viewport.x = 0;
        viewport.y = (h - w) / 2.0f;
        viewport.h = w;
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("App", "1.0", "com.example.app");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window* rawWindow = SDL_CreateWindow("App", 1920, 1200, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
    if (rawWindow == NULL) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    AppState->window.reset(rawWindow, AppDeleter);
    updateWindowSize();

    SDL_GPUDevice* rawGpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (rawGpuDevice == NULL) {
        SDL_Log("Couldn't create GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    AppState->gpuDevice.reset(rawGpuDevice, AppDeleter);
    
    if (!SDL_ClaimWindowForGPUDevice(AppState->gpuDevice.get(), AppState->window.get())) {
        SDL_Log("Couldn't claim window for GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetGPUSwapchainParameters(AppState->gpuDevice.get(), AppState->window.get(), SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR, SDL_GPU_PRESENTMODE_MAILBOX);

    initShaders();
    initPipeline();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            switch (event->key.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    return SDL_APP_SUCCESS;
                default:
                    break;
            }
        case SDL_EVENT_WINDOW_RESIZED:
            updateWindowSize();
            break;
    }
    return SDL_APP_CONTINUE;
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

    if (swapchainTexture != NULL) {
        SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
        colorTargetInfo.texture     = swapchainTexture;
        colorTargetInfo.clear_color = { 
            SDL_sinf(now * 3        ) * 0.5f + 0.5f,
            SDL_sinf(now * 3 + 2.09f) * 0.5f + 0.5f,
            SDL_sinf(now * 3 + 4.18f) * 0.5f + 0.5f,
        1.0f };
        colorTargetInfo.load_op     = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op    = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);
        
        SDL_SetGPUViewport(renderPass, &viewport);
        SDL_BindGPUGraphicsPipeline(renderPass, fillPipeline.get());
        SDL_DrawGPUPrimitives(renderPass, 6, 1, 0, 0);

        SDL_EndGPURenderPass(renderPass);
    }

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
        SDL_Log("Couldn't submit command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    // Release Vulkan context from SDL window
    SDL_ReleaseWindowFromGPUDevice(AppState->gpuDevice.get(), AppState->window.get());
}