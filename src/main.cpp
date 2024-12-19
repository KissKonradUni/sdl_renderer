#include "shader.hpp"
#include <SDL3/SDL_gpu.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

typedef struct SDL_AppState {
    SDL_Window*       window;
    SDL_GPUDevice* gpuDevice;
} SDL_AppState;
static SDL_AppState App;

static SDL_GPUShader* vertex;
static SDL_GPUShader* fragment;
static SDL_GPUGraphicsPipeline* fillPipeline;

void initShaders() {
    vertex   = LoadShader(App.gpuDevice, "RawTriangle.vert", 0, 0, 0, 0);
    fragment = LoadShader(App.gpuDevice, "SolidColor.frag" , 0, 0, 0, 0);
}

void initPipeline() {
    SDL_GPUColorTargetDescription colorTargetDescription = {
        .format = SDL_GetGPUSwapchainTextureFormat(App.gpuDevice, App.window)
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .vertex_shader   = vertex,
        .fragment_shader = fragment,
        .primitive_type  = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {
            .fill_mode  = SDL_GPU_FILLMODE_FILL,
            //.cull_mode  = SDL_GPU_CULLMODE_BACK,
            //.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
        },
        .target_info = {
            .color_target_descriptions = &colorTargetDescription,
            .num_color_targets         = 1,
        },
    };

    fillPipeline = SDL_CreateGPUGraphicsPipeline(App.gpuDevice, &pipelineCreateInfo);
    if (!fillPipeline) {
        SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
        return;
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("App", "1.0", "com.example.app");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    App.window = SDL_CreateWindow("App", 1920, 1200, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
    if (!App.window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    App.gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!App.gpuDevice) {
        SDL_Log("Couldn't create GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    if (!SDL_ClaimWindowForGPUDevice(App.gpuDevice, App.window)) {
        SDL_Log("Couldn't claim window for GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetGPUSwapchainParameters(App.gpuDevice, App.window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR, SDL_GPU_PRESENTMODE_MAILBOX);

    initShaders();
    initPipeline();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    } else if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_ESCAPE:
                return SDL_APP_SUCCESS;
            default:
                break;
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    //const double now = static_cast<double>(SDL_GetTicks()) / 1000.0;
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(App.gpuDevice);
    if (!commandBuffer) {
        SDL_Log("Couldn't acquire command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUViewport viewport = { 600.0f, 0.0f, 2000.0f, 2000.0f, 0.0f, 1.0f };

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_AcquireGPUSwapchainTexture(commandBuffer, App.window, &swapchainTexture, NULL, NULL)) {
        SDL_Log("Couldn't get swapchain texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (swapchainTexture != NULL) {
        SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
        colorTargetInfo.texture     = swapchainTexture;
        colorTargetInfo.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
        colorTargetInfo.load_op     = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op    = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);
        
        SDL_SetGPUViewport(renderPass, &viewport);
        SDL_BindGPUGraphicsPipeline(renderPass, fillPipeline);
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
    SDL_ReleaseWindowFromGPUDevice(App.gpuDevice, App.window);
    
    // Release Vulkan child objects
    SDL_ReleaseGPUShader(App.gpuDevice, vertex);
    SDL_ReleaseGPUShader(App.gpuDevice, fragment);
    SDL_ReleaseGPUGraphicsPipeline(App.gpuDevice, fillPipeline);

    // Release Vulkan device
    SDL_DestroyGPUDevice(App.gpuDevice);

    // Release SDL window
    SDL_DestroyWindow(App.window);
}