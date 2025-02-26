#include "lib/glad/glad.h"

#include "codex/mesh.hpp"
#include "echo/console.hpp"

#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/postprocess.h>

namespace Codex {

MeshData::~MeshData() {
    aiReleaseImport(scene);
}

Mesh::Mesh(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &m_vertexArrayObjectHandle);
    glBindVertexArray(m_vertexArrayObjectHandle);
    
    glGenBuffers(1, &m_vertexBufferObjectHandle);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjectHandle);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_indexBufferObjectHandle);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObjectHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // TODO: Add layout from here
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));

    glBindVertexArray(0);

    m_indexCount = indices.size();
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &m_vertexBufferObjectHandle);
    glDeleteBuffers(1, &m_indexBufferObjectHandle);
    glDeleteVertexArrays(1, &m_vertexArrayObjectHandle);

    Echo::log("Mesh destroyed.");
}

void Mesh::bind() const {
    glBindVertexArray(m_vertexArrayObjectHandle);
}

void Mesh::draw() const {
    bind();

    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
}

matrix4x4f Mesh::getModelMatrix() const {
    matrix4x4f translationMatrix = matrix4x4f::translation(position);
    matrix4x4f rotationMatrixX   = matrix4x4f::rotation(rotation.x, vector4f::right());
    matrix4x4f rotationMatrixY   = matrix4x4f::rotation(rotation.y, vector4f::up());
    matrix4x4f rotationMatrixZ   = matrix4x4f::rotation(rotation.z, vector4f::front());
    matrix4x4f scaleMatrix       = matrix4x4f::scale(scale);

    return scaleMatrix * rotationMatrixZ * rotationMatrixY * rotationMatrixX * translationMatrix;
}

std::shared_ptr<MeshData> Mesh::loadMeshDataFromFile(const std::string& filename) {
    auto scene = aiImportFile(filename.c_str(), 
        aiProcess_Triangulate | aiProcess_CalcTangentSpace
    );

    if (!scene) {
        std::string message = "Couldn't load mesh: " + filename;
        Echo::warn(message);
        return nullptr;
    }

    std::string message = "Loaded mesh: " + filename;
    Echo::log(message);

    return std::make_unique<MeshData>(scene);
}

std::shared_ptr<Mesh> Mesh::loadMeshFromFile(const std::string& filename) {
    auto data = loadMeshDataFromFile(filename);
    return processMeshData(data);
}

std::shared_ptr<Mesh> Mesh::processMeshData(std::shared_ptr<MeshData> data) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int indexOffset = 0;

    bool hasNormals  = true;
    bool hasTangents = true;
    bool hasUVs      = true;

    for (unsigned int i = 0; i < data->scene->mNumMeshes; i++) {
        auto mesh = data->scene->mMeshes[i];

        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            vertices.push_back(mesh->mVertices[j].x);
            vertices.push_back(mesh->mVertices[j].y);
            vertices.push_back(mesh->mVertices[j].z);

            if (mesh->HasNormals()) {
                vertices.push_back(mesh->mNormals[j].x);
                vertices.push_back(mesh->mNormals[j].y);
                vertices.push_back(mesh->mNormals[j].z);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                hasNormals = false;
            }

            if (mesh->HasTangentsAndBitangents()) {
                vertices.push_back(mesh->mTangents[j].x);
                vertices.push_back(mesh->mTangents[j].y);
                vertices.push_back(mesh->mTangents[j].z);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                hasTangents = false;
            }

            if (mesh->HasTextureCoords(0)) {
                vertices.push_back(mesh->mTextureCoords[0][j].x);
                vertices.push_back(mesh->mTextureCoords[0][j].y);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                hasUVs = false;
            }
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            auto face = mesh->mFaces[j];

            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k] + indexOffset);
            }
        }

        indexOffset += mesh->mNumVertices;
    }

    if (!hasNormals) {
        Echo::warn(std::string("Mesh doesn't have normals."));
    }
    if (!hasTangents) {
        Echo::warn(std::string("Mesh doesn't have tangents."));
    }
    if (!hasUVs) {
        Echo::warn(std::string("Mesh doesn't have UVs."));
    }

    return std::make_shared<Mesh>(vertices, indices);
}

}; // namespace Codex