#include "shader.hpp"

SDL_GPUShader* LoadShader(
    SDL_GPUDevice* device,
    std::string shaderFilename,
    Uint32 samplerCount,
    Uint32 uniformCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount
) {
    SDL_GPUShaderStage stage;
    if (SDL_strstr(shaderFilename.c_str(), ".vert")) {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    } else if (SDL_strstr(shaderFilename.c_str(), ".frag")) {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    } else {
        SDL_Log("Invalid shader file extension: %s", shaderFilename.c_str());
        return NULL;
    }

    std::string basePath = SDL_GetBasePath();
    std::string fullPath;

    SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;

    std::string entryPoint;

    if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
        // Vulkan
        fullPath = basePath + "../assets/shaders/compiled/SPIRV/" + shaderFilename + ".spv";
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entryPoint = "main";
    } else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
        // Metal
        fullPath = basePath + "../assets/shaders/compiled/MSL/" + shaderFilename + ".msl";
        format = SDL_GPU_SHADERFORMAT_MSL;
        entryPoint = "main0";
    } else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
        // DirectX
        fullPath = basePath + "../assets/shaders/compiled/DXIL/" + shaderFilename + ".dxil";
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entryPoint = "main";
    } else {
        SDL_Log("No supported shader formats found!");
        return NULL;
    }

    size_t codeSize;
    void* code = SDL_LoadFile(fullPath.c_str(), &codeSize);
    if (!code) {
        SDL_Log("Couldn't load shader file: %s", SDL_GetError());
        return NULL;
    }

    SDL_GPUShaderCreateInfo info = {
        .code_size = codeSize,
        .code = static_cast<const Uint8*>(code),
        .entrypoint = entryPoint.c_str(),
        .format = format,
        .stage = stage,
        .num_samplers = samplerCount,
        .num_storage_textures = storageTextureCount,
        .num_storage_buffers = storageBufferCount,
        .num_uniform_buffers = uniformCount
    };

    SDL_GPUShader* shaderPtr = SDL_CreateGPUShader(device, &info);
    if (!shaderPtr) {
        SDL_Log("Couldn't create shader: %s", SDL_GetError());
        SDL_free(code);
        return NULL;
    }

    SDL_free(code);
    return shaderPtr;
}