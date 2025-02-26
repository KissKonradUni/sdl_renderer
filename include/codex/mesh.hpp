#pragma once

#include <SDL3/SDL.h>
#include <assimp/scene.h>

#include <vector>
#include <memory>

#include "floatmath.hpp"

namespace Codex {

struct MeshData {
    const aiScene* scene;

    ~MeshData();
};

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

    static std::shared_ptr<MeshData> loadMeshDataFromFile(const std::string& filename);
    static std::shared_ptr<Mesh> loadMeshFromFile(const std::string& filename);
    static std::shared_ptr<Mesh> processMeshData(std::shared_ptr<MeshData> data);
protected:
    unsigned int m_vertexBufferObjectHandle;
    unsigned int m_indexBufferObjectHandle;
    unsigned int m_vertexArrayObjectHandle;

    unsigned int m_indexCount;

    void bind() const;
};

}; // namespace Codex