#include "codex/packedmesh.hpp"
#include "echo/console.hpp"

#include <lib/glad/glad.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Codex {

PackedMesh::PackedMesh() {
    glGenVertexArrays(1, &m_vertexArrayObjectHandle);
    glBindVertexArray(m_vertexArrayObjectHandle);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));

    glGenBuffers(1, &m_ssboVertexHandle);   
    glGenBuffers(1, &m_ssboIndexHandle);
    glGenBuffers(1, &m_ssboRecordHandle);
    glGenBuffers(1, &m_ssboInstanceHandle);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboVertexHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, PACKED_MESH_ALLOCATION_SIZE * sizeof(float), NULL, GL_STATIC_DRAW);
    m_vertexBufferSize = PACKED_MESH_ALLOCATION_SIZE;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboIndexHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, PACKED_MESH_ALLOCATION_SIZE * sizeof(unsigned int), NULL, GL_STATIC_DRAW);
    m_indexBufferSize = PACKED_MESH_ALLOCATION_SIZE;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboRecordHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, PACKED_MESH_ALLOCATION_SIZE * sizeof(PackedMeshRecord), NULL, GL_STATIC_DRAW);
    m_recordBufferSize = PACKED_MESH_ALLOCATION_SIZE;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboInstanceHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, PACKED_MESH_ALLOCATION_SIZE * sizeof(PackedMeshInstance), NULL, GL_DYNAMIC_DRAW);
    m_instanceBufferSize = PACKED_MESH_ALLOCATION_SIZE;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindVertexArray(0);

    m_vertexCount = 0;
    m_indexCount = 0;

    Echo::log("Packed mesh created.");
}
PackedMesh::~PackedMesh() {
    glDeleteBuffers(1, &m_ssboVertexHandle);
    glDeleteBuffers(1, &m_ssboIndexHandle);
    glDeleteBuffers(1, &m_ssboRecordHandle);
    glDeleteBuffers(1, &m_ssboInstanceHandle);

    Echo::log("Packed mesh destroyed.");
}

void PackedMesh::addMesh(PackedMeshData& data) {
    extendVertexBuffer(m_vertexCount + data.vertices.size());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboVertexHandle);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_vertexCount * sizeof(float), data.vertices.size() * sizeof(float), data.vertices.data());

    extendIndexBuffer(m_indexCount + data.indices.size());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboIndexHandle);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_indexCount * sizeof(unsigned int), data.indices.size() * sizeof(unsigned int), data.indices.data());
    
    PackedMeshRecord record;
    record.vertexOffset = m_vertexCount;
    record.indexOffset = m_indexCount;
    record.vertexCount = data.vertices.size();
    record.indexCount = data.indices.size();

    extendRecordBuffer(m_records.size() + 1);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboRecordHandle);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_records.size() * sizeof(PackedMeshRecord), sizeof(PackedMeshRecord), &record);
    m_records.push_back(record);

    m_vertexCount += data.vertices.size();
    m_indexCount += data.indices.size();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
void PackedMesh::addInstance(transformf* transform, unsigned int recordIndex) {
    extendInstanceBuffer(m_instances.size() + 1);
    PackedMeshInstance instance;
    instance.modelMatrix = transform->getModelMatrix();
    instance.recordIndex = recordIndex;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboInstanceHandle);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_instances.size() * sizeof(PackedMeshInstance), sizeof(PackedMeshInstance), &instance);
    m_instances.push_back(instance);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
void PackedMesh::bind() const {
    glBindVertexArray(m_vertexArrayObjectHandle);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboVertexHandle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboIndexHandle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ssboRecordHandle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_ssboInstanceHandle);
}
void PackedMesh::draw() const {
    bind();
}

void PackedMesh::loadMeshDataFromFile(const std::string& filename, PackedMeshData* data) {
    auto scene = aiImportFile(filename.c_str(), 
        aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords
    );

    if (!scene) {
        std::string message = "Couldn't load mesh: " + filename;
        Echo::warn(message);
        data = nullptr;
        return;
    }

    std::string message = "Loaded mesh: " + filename;
    Echo::log(message);

    // TODO: maybe extend to multiple meshes, right now load the first one.
    auto rootNode = scene->mRootNode;
    auto mesh = scene->mMeshes[rootNode->mMeshes[0]];
    for (int i = 0; i < mesh->mNumVertices; i++) {
        data->vertices.push_back(mesh->mVertices[i].x);
        data->vertices.push_back(mesh->mVertices[i].y);
        data->vertices.push_back(mesh->mVertices[i].z);

        data->vertices.push_back(mesh->mNormals[i].x);
        data->vertices.push_back(mesh->mNormals[i].y);
        data->vertices.push_back(mesh->mNormals[i].z);

        data->vertices.push_back(mesh->mTangents[i].x);
        data->vertices.push_back(mesh->mTangents[i].y);
        data->vertices.push_back(mesh->mTangents[i].z);

        data->vertices.push_back(mesh->mTextureCoords[0][i].x);
        data->vertices.push_back(mesh->mTextureCoords[0][i].y);
    }

    for (int i = 0; i < mesh->mNumFaces; i++) {
        auto face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++) {
            data->indices.push_back(face.mIndices[j]);
        }
    }

    aiReleaseImport(scene);

    Echo::log("Mesh data processed.");
}

void extendSSBO(unsigned int* handle, unsigned int* size, int min) {
    // Check if we need to extend the buffer
    if ((*size) >= min) {
        return;
    }

    // Calculate the new size
    unsigned int newSize = (*size);
    while (newSize < min) {
        newSize += PACKED_MESH_ALLOCATION_SIZE;
    }

    // Create a new buffer
    unsigned int newHandle;
    glGenBuffers(1, &newHandle);

    // Copy the data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, newHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, newSize, NULL, GL_STATIC_DRAW);
    glCopyNamedBufferSubData(*handle, newHandle, 0, 0, *size);

    // Delete the old buffer
    glDeleteBuffers(1, handle);

    // Update the handle
    *handle = newHandle;
    *size = newSize;

    // Unbind the buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    Echo::log(std::string("Extended SSBO to ") + std::to_string(newSize));
}

void PackedMesh::extendVertexBuffer(int min) {
    extendSSBO(&m_ssboVertexHandle, &m_vertexBufferSize, min);
}
void PackedMesh::extendIndexBuffer(int min) {
    extendSSBO(&m_ssboIndexHandle, &m_indexBufferSize, min);
}
void PackedMesh::extendRecordBuffer(int min) {
    extendSSBO(&m_ssboRecordHandle, &m_recordBufferSize, min);
}
void PackedMesh::extendInstanceBuffer(int min) {
    extendSSBO(&m_ssboInstanceHandle, &m_instanceBufferSize, min);
}

}