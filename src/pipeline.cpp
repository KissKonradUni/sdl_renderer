#include "pipeline.hpp"

std::shared_ptr<SDL_GPUGraphicsPipeline> createPipeline(
    std::shared_ptr<SDL_GPUShader> vertexShader,
    std::shared_ptr<SDL_GPUShader> fragmentShader
) {
    SDL_GPUColorTargetDescription colorTargetDescription = {
        .format = SDL_GetGPUSwapchainTextureFormat(AppState->gpuDevice.get(), AppState->window.get())
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .vertex_shader   = vertexShader.get(),
        .fragment_shader = fragmentShader.get(),
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

    SDL_GPUGraphicsPipeline* rawPipeline = SDL_CreateGPUGraphicsPipeline(AppState->gpuDevice.get(), &pipelineCreateInfo);
    if (rawPipeline == NULL) {
        SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
        return NULL;
    }

    return std::shared_ptr<SDL_GPUGraphicsPipeline>(rawPipeline, PipelineDeleter);
}