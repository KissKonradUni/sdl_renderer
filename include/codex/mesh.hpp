#pragma once

#include <SDL3/SDL.h>
#include <assimp/scene.h>

#include <vector>
#include <memory>

#include "floatmath.hpp"

namespace Codex {

struct SceneData {
    const aiScene* scene;

    ~SceneData();
};

class Mesh {
public:
    transformf transform;

    // TODO: Add layout
    Mesh(std::vector<float>& vertices, std::vector<unsigned int>& indices/*, layout*/);
    ~Mesh();

    void draw() const;

    static std::shared_ptr<SceneData> loadSceneDataFromFile(const std::string& filename);
    static std::shared_ptr<Mesh> loadSceneFromFile(const std::string& filename);
    static std::shared_ptr<Mesh> processCombinedSceneData(const std::shared_ptr<SceneData> data);
    static std::shared_ptr<std::vector<std::shared_ptr<Mesh>>> processScene(const std::shared_ptr<SceneData> data, transformf& parentTransform);
protected:
    static void recursiveProcessScene(const aiScene* scene, const aiNode* node, std::vector<std::shared_ptr<Mesh>>& meshes, transformf& parentTransform);
    static std::shared_ptr<Mesh> processMesh(const aiMesh* mesh);

    unsigned int m_vertexBufferObjectHandle;
    unsigned int m_indexBufferObjectHandle;
    unsigned int m_vertexArrayObjectHandle;

    unsigned int m_indexCount;

    void bind() const;
};

}; // namespace Codex