#pragma once

#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include <SDL3/SDL.h>

#include <vector>

#include "floatmath.hpp"

class Mesh {
public:
    vector4f position;
    vector4f rotation;
    vector4f scale = vector4f::one();

    // TODO: Add layout
    Mesh(std::vector<float>& vertices, std::vector<unsigned int>& indices/*, layout*/);
    ~Mesh();

    void draw() const;
    matrix4x4f getModelMatrix() const;
protected:
    unsigned int vertexBufferObjectHandle;
    unsigned int indexBufferObjectHandle;
    unsigned int vertexArrayObjectHandle;

    unsigned int indexCount;

    void bind() const;
};