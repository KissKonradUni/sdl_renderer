#include "cinder.hpp"

#include "codex/mesh.hpp"
#include "codex/library.hpp"

#include <glad.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace codex {

Mesh::Mesh(MeshPart* data, std::vector<Layout>& layout) {
    m_data = nullptr;
    m_node = nullptr;
    m_initialized = false;
    m_runtimeResource = true;

    m_layout = layout;
    uploadData(data);
    m_data.reset();

    if (data == nullptr) {
        cinder::warn("Tried creating mesh with null data.");
        return;
    }
}

Mesh::Mesh() {
    m_data = nullptr;
    m_node = nullptr;
    m_runtimeResource = true;
    m_initialized = false;

    cinder::log("Mesh placeholder created.");
}
bool Mesh::m_suppressDestroyMessage = false;

Mesh::~Mesh() {
    if (m_vertexBufferObjectHandle > 0)
        glDeleteBuffers(1, &m_vertexBufferObjectHandle);
    if (m_indexBufferObjectHandle > 0)
        glDeleteBuffers(1, &m_indexBufferObjectHandle);
    if (m_vertexArrayObjectHandle > 0)
        glDeleteVertexArrays(1, &m_vertexArrayObjectHandle);

    if (!Mesh::m_suppressDestroyMessage) {
        cinder::log("Mesh destroyed... extra messages supressed.");
        Mesh::m_suppressDestroyMessage = true;
    }
}

void Mesh::loadData(const FileNode* node) {
    if (m_initialized) {
        cinder::warn("Mesh data already loaded.");
        return;
    }

    if (node == nullptr) {
        cinder::warn("Tried loading mesh data from invalid node.");
        return;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile((Library::instance().getAssetsRoot() / node->path).string(),
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace
    );

    if (scene == nullptr) {
        cinder::error(std::string("Failed to load mesh data: ") + importer.GetErrorString());
        return;
    }

    // TODO: Might want to change the layout, let's hardcode it for now
    std::vector<Layout> layout = {
        Layout(Layout::Type::FLOAT3), // POSITION
        Layout(Layout::Type::FLOAT3), // NORMAL
        Layout(Layout::Type::FLOAT3), // TANGENT
        Layout(Layout::Type::FLOAT2), // UV
    };

    // Initialize the mesh data
    m_data = std::unique_ptr<MeshData>(new MeshData());
    m_data->layout = layout;
    m_node = node;
    m_layout = layout;

    // TODO: Remake it in a recursive way
    aiNode* rootNode = scene->mRootNode;
    loadDataRecursive(m_data.get(), rootNode, scene);

    importer.FreeScene();
    m_runtimeResource = false;
    cinder::log("Loaded mesh data from file: " + node->path.string());
}

void Mesh::loadDataRecursive(MeshData* data, const aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        MeshPart* meshPart = new MeshPart();
        meshPart->vertices.reserve(mesh->mNumVertices * Layout::calculateStride(m_layout));
        meshPart->indices.reserve(mesh->mNumFaces * 3);

        for (unsigned int k = 0; k < mesh->mNumVertices; k++) {
            aiVector3D vertex = mesh->mVertices[k];
            meshPart->vertices.push_back(vertex.x);
            meshPart->vertices.push_back(vertex.y);
            meshPart->vertices.push_back(vertex.z);

            aiVector3D normal = mesh->mNormals[k];
            meshPart->vertices.push_back(normal.x);
            meshPart->vertices.push_back(normal.y);
            meshPart->vertices.push_back(normal.z);

            aiVector3D tangent = mesh->mTangents[k];
            meshPart->vertices.push_back(tangent.x);
            meshPart->vertices.push_back(tangent.y);
            meshPart->vertices.push_back(tangent.z);

            aiVector3D uv = mesh->mTextureCoords[0][k];
            meshPart->vertices.push_back(uv.x);
            meshPart->vertices.push_back(uv.y);
        }

        for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
            aiFace face = mesh->mFaces[k];
            for (unsigned int l = 0; l < face.mNumIndices; l++) {
                meshPart->indices.push_back(face.mIndices[l]);
            }
        }

        meshPart->vertexCount = mesh->mNumVertices;
        meshPart->indexCount = mesh->mNumFaces * 3;

        data->meshParts.push_back(std::unique_ptr<MeshPart>(meshPart));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        loadDataRecursive(data, node->mChildren[i], scene);
    }
}

void Mesh::loadResource() {
    if (m_initialized) {
        cinder::warn("Mesh already initialized.");
        return;
    }

    if (m_data == nullptr) {
        cinder::error("Mesh data is null.");
        return;
    }

    auto baseName = m_node->name;
    auto folderNode = Library::instance().requestRuntimeNode((Library::instance().getAssetsRoot() / "runtime" / baseName).string());
    if (folderNode == nullptr) {
        cinder::warn("Failed to create runtime folder node.");
        return;
    }
    folderNode->type = FileType::MESH_FILE;
    folderNode->isDirectory = true;
    folderNode->name = baseName;

    for (int i = 0; i < m_data->meshParts.size(); i++) {
        if (i == 0)
            continue;

        MeshPart* meshPart = m_data->meshParts[i].get();
        
        std::string partName = std::format("part_{:0>3}.mesh", i);
        auto meshNode = Library::instance().requestRuntimeNode((Library::instance().getAssetsRoot() / "runtime" / baseName / partName).string(), folderNode);
        if (meshNode == nullptr) {
            cinder::warn("Failed to create runtime mesh node.");
            return;
        }
        meshNode->type = FileType::MESH_PART;
        meshNode->isDirectory = false;
        meshNode->name = partName;
        meshNode->extension = "mesh";

        Mesh* mesh = new Mesh(meshPart, m_layout);
        mesh->m_node = meshNode;
        m_meshParts.push_back(mesh);

        Library::instance().registerRuntimeResource(mesh);
    }

    MeshPart* meshPart = m_data->meshParts[0].get();
    uploadData(meshPart);

    cinder::log("Mesh created with " + std::to_string(m_data->meshParts.size()) + " parts.");
    m_data.reset();
}

void Mesh::uploadData(MeshPart* data) {
    glGenVertexArrays(1, &m_vertexArrayObjectHandle);
    glBindVertexArray(m_vertexArrayObjectHandle);

    glGenBuffers(1, &m_vertexBufferObjectHandle);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjectHandle);
    glBufferData(GL_ARRAY_BUFFER, data->vertices.size() * sizeof(float), data->vertices.data(), GL_STATIC_DRAW);

    for (int i = 0; i < m_layout.size(); i++) {
        glEnableVertexAttribArray(i);
        uint8_t stride = Layout::calculateStride(m_layout);
        uint64_t offset = Layout::calculateOffset(m_layout, i);
        glVertexAttribPointer(i, m_layout[i].getSize() / sizeof(float), GL_FLOAT, GL_FALSE, stride, (void*) offset);
    }

    glGenBuffers(1, &m_indexBufferObjectHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObjectHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->indices.size() * sizeof(uint32_t), data->indices.data(), GL_STATIC_DRAW);

    m_vertexCount = data->vertexCount;
    m_indexCount = data->indexCount;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_initialized = true;
}

void Mesh::draw() const {
    if (!m_initialized) {
        return;
    }

    glBindVertexArray(m_vertexArrayObjectHandle);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    for (const auto& parts : m_meshParts) {
        parts->draw();
    }
}

}; // namespace codex