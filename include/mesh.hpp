#pragma once

#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include <SDL3/SDL.h>

#include "app.hpp"

class Mesh {
public:
    Mesh();
    ~Mesh();

    void loadMesh(const std::string& filename);
    void render(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass);

protected:
    const aiScene* scene;
};