#pragma once

#include <string>
#include <vector>

#include "floatmath.hpp"

namespace Codex {

#define PACKED_MESH_ALLOCATION_SIZE 256

struct PackedMeshData {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

struct PackedMeshRecord {
    unsigned int vertexOffset;
    unsigned int indexOffset;

    unsigned int vertexCount;
    unsigned int indexCount;
};

struct PackedMeshInstance {
    matrix4x4f modelMatrix;
    unsigned int recordIndex;

    transformf* transform;
};

class PackedMesh {
public:
    PackedMesh();
    ~PackedMesh();

    void addMesh(PackedMeshData& data);
    void addInstance(transformf* modelMatrix, unsigned int recordIndex);
    void bind() const;
    void draw() const;

    static void loadMeshDataFromFile(const std::string& filename, PackedMeshData* data);
protected:
    std::vector<PackedMeshRecord> m_records = {};
    std::vector<PackedMeshInstance> m_instances = {};

    unsigned int m_vertexArrayObjectHandle;
    
    unsigned int m_ssboVertexHandle;
    unsigned int m_ssboIndexHandle;
    unsigned int m_ssboRecordHandle;
    unsigned int m_ssboInstanceHandle;

    unsigned int m_vertexBufferSize;
    unsigned int m_indexBufferSize;
    unsigned int m_recordBufferSize;
    unsigned int m_instanceBufferSize;

    unsigned int m_vertexCount;
    unsigned int m_indexCount;

    void extendVertexBuffer(int min);
    void extendIndexBuffer(int min);
    void extendRecordBuffer(int min);
    void extendInstanceBuffer(int min);
};

}; // namespace Codex