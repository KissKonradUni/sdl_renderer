#pragma once

#include "app.hpp"

#include <SDL3/SDL.h>

#include <memory>
#include <string>

class SDL_ShaderDeleter {
public:
    void operator()(SDL_GPUShader* shader) {
        SDL_ReleaseGPUShader(AppState->gpuDevice.get(), shader);
        SDL_Log("Shader released");
    }
};
extern SDL_ShaderDeleter ShaderDeleter;

std::shared_ptr<SDL_GPUShader> LoadShader(
    std::shared_ptr<SDL_GPUDevice> device,
    std::string shaderFilename,
    Uint32 samplerCount,
    Uint32 uniformCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount
);