#pragma once

#include <SDL3/SDL.h>

#include <string>

SDL_GPUShader* LoadShader(
    SDL_GPUDevice* device,
    std::string shaderFilename,
    Uint32 samplerCount,
    Uint32 uniformCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount
);