#include "mesh.hpp"

void Mesh::loadMesh(const std::string& filename) {
    scene = aiImportFile(filename.c_str(), aiProcess_Triangulate);

    if (!scene) {
        SDL_Log("Couldn't load mesh: %s", filename.c_str());
        return;
    }

    SDL_Log("Loaded mesh: %s", filename.c_str());

    throw std::runtime_error("Not implemented");
}

void render(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass) {
    SDL_DrawGPUIndexedPrimitives(renderPass, 0, 1, 0, 0, 0);
}

Mesh::~Mesh() {
    aiReleaseImport(scene);
}