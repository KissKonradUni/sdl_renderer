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
            .cull_mode  = SDL_GPU_CULLMODE_NONE,
            //.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
        },
        .depth_stencil_state = {
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            /*
            .back_stencil_state = {
                .fail_op = SDL_GPU_STENCILOP_REPLACE,
                .pass_op = SDL_GPU_STENCILOP_KEEP,
                .depth_fail_op = SDL_GPU_STENCILOP_KEEP,
                .compare_op = SDL_GPU_COMPAREOP_NEVER,
            },
            .front_stencil_state = {
                .fail_op = SDL_GPU_STENCILOP_REPLACE,
                .pass_op = SDL_GPU_STENCILOP_KEEP,
                .depth_fail_op = SDL_GPU_STENCILOP_KEEP,
                .compare_op = SDL_GPU_COMPAREOP_NEVER,
            },
            .write_mask = 0xFF,
            .enable_depth_test = true,
            .enable_depth_write = true,
            .enable_stencil_test = true
            */
            .write_mask = 0,
            .enable_depth_test = false,
            .enable_depth_write = false,
            .enable_stencil_test = false
        },
        .target_info = {
            .color_target_descriptions = &colorTargetDescription,
            .num_color_targets         = 1,
            /*
            .depth_stencil_format      = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
            .has_depth_stencil_target  = true,
            */
        },
    };

    SDL_GPUGraphicsPipeline* rawPipeline = SDL_CreateGPUGraphicsPipeline(AppState->gpuDevice.get(), &pipelineCreateInfo);
    if (rawPipeline == NULL) {
        SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
        return NULL;
    }

    return std::shared_ptr<SDL_GPUGraphicsPipeline>(rawPipeline, PipelineDeleter);
}