#include "lib/glad/glad.h"

#include "codex/mesh.hpp"
#include "echo/console.hpp"

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/postprocess.h>

namespace Codex {

Mesh::Mesh(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    glGenVertexArrays(1, &vertexArrayObjectHandle);
    glBindVertexArray(vertexArrayObjectHandle);
    
    glGenBuffers(1, &vertexBufferObjectHandle);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectHandle);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &indexBufferObjectHandle);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObjectHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // TODO: Add layout from here
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    indexCount = indices.size();
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &vertexBufferObjectHandle);
    glDeleteBuffers(1, &indexBufferObjectHandle);
    glDeleteVertexArrays(1, &vertexArrayObjectHandle);

    Echo::log("Mesh destroyed.");
}

void Mesh::bind() const {
    glBindVertexArray(vertexArrayObjectHandle);
}

void Mesh::draw() const {
    bind();

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

matrix4x4f Mesh::getModelMatrix() const {
    matrix4x4f translationMatrix = matrix4x4f::translation(position);
    matrix4x4f rotationMatrixX   = matrix4x4f::rotation(rotation.x, vector4f::right());
    matrix4x4f rotationMatrixY   = matrix4x4f::rotation(rotation.y, vector4f::up());
    matrix4x4f rotationMatrixZ   = matrix4x4f::rotation(rotation.z, vector4f::front());
    matrix4x4f scaleMatrix       = matrix4x4f::scale(scale);

    return scaleMatrix * rotationMatrixZ * rotationMatrixY * rotationMatrixX * translationMatrix;
}

std::unique_ptr<Mesh> Mesh::loadMeshFromFile(const std::string& filename) {
    auto scene = aiImportFile(filename.c_str(), aiProcess_Triangulate);

    if (!scene) {
        std::string message = "Couldn't load mesh: " + filename;
        Echo::warn(message);
        return nullptr;
    }

    std::string message = "Loaded mesh: " + filename;
    Echo::log(message);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        auto mesh = scene->mMeshes[i];

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
            }

            if (mesh->HasTextureCoords(0)) {
                vertices.push_back(mesh->mTextureCoords[0][j].x);
                vertices.push_back(mesh->mTextureCoords[0][j].y);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            auto face = mesh->mFaces[j];

            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k]);
            }
        }
    }

    // Oops, was leaking quite a bit of memory here
    aiReleaseImport(scene);

    return std::make_unique<Mesh>(vertices, indices);
}

}; // namespace Codex