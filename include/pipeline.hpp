#pragma once

#include "SDL3/SDL_gpu.h"
#include "app.hpp"

#include <SDL3/SDL.h>

#include <memory>

class SDL_PipelineDeleter {
public:
    void operator()(SDL_GPUGraphicsPipeline* pipeline) {
        SDL_ReleaseGPUGraphicsPipeline(AppState->gpuDevice.get(), pipeline);
    }
};
extern SDL_PipelineDeleter PipelineDeleter;

std::shared_ptr<SDL_GPUGraphicsPipeline> CreatePipeline(
    std::shared_ptr<SDL_GPUShader> vertexShader,
    std::shared_ptr<SDL_GPUShader> fragmentShader
);