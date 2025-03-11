#include "lib/glad/glad.h"

#include "codex/mesh.hpp"
#include "echo/console.hpp"

#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Codex {

SceneData::~SceneData() {
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

std::shared_ptr<SceneData> Mesh::loadSceneDataFromFile(const std::string& filename) {
    auto scene = aiImportFile(filename.c_str(), 
        aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_GenUVCoords
    );

    if (!scene) {
        std::string message = "Couldn't load mesh: " + filename;
        Echo::warn(message);
        return nullptr;
    }

    std::string message = "Loaded mesh: " + filename;
    Echo::log(message);

    return std::make_unique<SceneData>(scene);
}

std::shared_ptr<Mesh> Mesh::loadSceneFromFile(const std::string& filename) {
    auto data = loadSceneDataFromFile(filename);
    return processCombinedSceneData(data);
}

std::shared_ptr<Mesh> Mesh::processCombinedSceneData(const std::shared_ptr<SceneData> data) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    int indexOffset = 0;

    for (unsigned int i = 0; i < data->scene->mNumMeshes; i++) {
        auto mesh = data->scene->mMeshes[i];

        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            vertices.push_back(mesh->mVertices[j].x);
            vertices.push_back(mesh->mVertices[j].y);
            vertices.push_back(mesh->mVertices[j].z);

            vertices.push_back(mesh->mNormals[j].x);
            vertices.push_back(mesh->mNormals[j].y);
            vertices.push_back(mesh->mNormals[j].z);

            vertices.push_back(mesh->mTangents[j].x);
            vertices.push_back(mesh->mTangents[j].y);
            vertices.push_back(mesh->mTangents[j].z);

            vertices.push_back(mesh->mTextureCoords[0][j].x);
            vertices.push_back(mesh->mTextureCoords[0][j].y);
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            auto face = mesh->mFaces[j];

            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indices.push_back(face.mIndices[k] + indexOffset);
            }
        }

        indexOffset += mesh->mNumVertices;
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<std::vector<std::shared_ptr<Mesh>>> Mesh::processScene(const std::shared_ptr<SceneData> data, transformf& parentTransform) {
    std::shared_ptr<std::vector<std::shared_ptr<Mesh>>> meshes = std::make_shared<std::vector<std::shared_ptr<Mesh>>>();
    meshes->reserve(data->scene->mNumMeshes);
    
    auto rootNode = data->scene->mRootNode;
    recursiveProcessScene(data->scene, rootNode, *meshes, parentTransform);

    return meshes;
}

void Mesh::recursiveProcessScene(const aiScene* scene, const aiNode* node, std::vector<std::shared_ptr<Mesh>>& meshes, transformf& parentTransform) {
    aiVector3f position, rotation, scale;
    node->mTransformation.Decompose(scale, rotation, position);

    transformf transform(parentTransform);

    transform.setPosition(vector4f(position.x, position.y, position.z, 0.0f));
    transform.setRotation(vector4f(rotation.x, rotation.y, rotation.z, 0.0f));
    transform.setScale(vector4f(scale.x, scale.y, scale.z, 1.0f));
    
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        auto mesh = scene->mMeshes[node->mMeshes[i]];

        auto processedMesh = processMesh(mesh);
        processedMesh->transform = transform;

        meshes.push_back(processedMesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        recursiveProcessScene(scene, node->mChildren[i], meshes, transform);
    }
}

std::shared_ptr<Mesh> Mesh::processMesh(const aiMesh* mesh) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);

        vertices.push_back(mesh->mTangents[i].x);
        vertices.push_back(mesh->mTangents[i].y);
        vertices.push_back(mesh->mTangents[i].z);

        vertices.push_back(mesh->mTextureCoords[0][i].x);
        vertices.push_back(mesh->mTextureCoords[0][i].y);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        auto face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}

}; // namespace Codex