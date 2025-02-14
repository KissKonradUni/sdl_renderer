#include "glad/glad.h"

#include "mesh.hpp"

#include <stdexcept>

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    indexCount = indices.size();
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &vertexBufferObjectHandle);
    glDeleteBuffers(1, &indexBufferObjectHandle);
    glDeleteVertexArrays(1, &vertexArrayObjectHandle);
}

void Mesh::bind() {
    glBindVertexArray(vertexArrayObjectHandle);
}

void Mesh::draw() {
    bind();

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}




void loadMesh(const std::string& filename) {
    auto scene = aiImportFile(filename.c_str(), aiProcess_Triangulate);

    if (!scene) {
        SDL_Log("Couldn't load mesh: %s", filename.c_str());
        return;
    }

    SDL_Log("Loaded mesh: %s", filename.c_str());

    throw std::runtime_error("Not implemented");
}

/*
Mesh::~Mesh() {
    aiReleaseImport(scene);
}
*/