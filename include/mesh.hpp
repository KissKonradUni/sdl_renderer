#pragma once

#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include <SDL3/SDL.h>

#include <vector>

class Mesh {
public:
    // TODO: Add layout
    Mesh(std::vector<float>& vertices, std::vector<unsigned int>& indices/*, layout*/);
    ~Mesh();

    void draw();
protected:
    unsigned int vertexBufferObjectHandle;
    unsigned int indexBufferObjectHandle;
    unsigned int vertexArrayObjectHandle;

    unsigned int indexCount;

    void bind();
};