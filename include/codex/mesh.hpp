#pragma once

#include "codex/resource.hpp"

#include <assimp/scene.h>
#include <cstdint>
#include <glad.h>

namespace codex {

// TODO: totally rework, it's garbage
struct Layout {
public:
    enum Type : uint8_t {
        FLOAT1 =     sizeof(float),
        FLOAT2 = 2 * sizeof(float),
        FLOAT3 = 3 * sizeof(float),
        FLOAT4 = 4 * sizeof(float),
        UINT1  =     sizeof(uint32_t),
        UINT2  = 2 * sizeof(uint32_t),
        UINT3  = 3 * sizeof(uint32_t),
        UINT4  = 4 * sizeof(uint32_t)
    };

    Layout(Type type) : m_type(type) {}

    inline constexpr const Type getType() const {
        return m_type;
    }

    inline constexpr const uint8_t getSize() const {
        return m_type;
    }

    static inline constexpr const uint8_t calculateOffset(std::vector<Layout>& layout, int index) {
        uint8_t offset = 0;
        for (int i = 0; i < index; i++) {
            offset += layout[i].m_type;
        }
        return offset;
    }

    static inline constexpr const uint8_t calculateStride(std::vector<Layout>& layout) {
        return calculateOffset(layout, layout.size());
    }

protected:
    Type m_type;
};

struct MeshPart {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    uint32_t vertexCount = -1;
    uint32_t indexCount  = -1;

    MeshPart() = default;
    ~MeshPart() = default;
};

struct MeshData {
    std::vector<Layout> layout;
    std::vector<std::unique_ptr<MeshPart>> meshParts;
};

class Mesh : public IResource<MeshData> {
public:
    Mesh(MeshPart* data, std::vector<Layout>& layout);
    Mesh();
    ~Mesh();

    void loadData(const FileNode* node) override;
    void loadDataRecursive(MeshData* data, const aiNode* node, const aiScene* scene);
    void loadResource() override;

    void draw() const;
protected:
    std::vector<Layout> m_layout;

    uint32_t m_vertexArrayObjectHandle  = -1;
    uint32_t m_vertexBufferObjectHandle = -1;
    uint32_t m_indexBufferObjectHandle  = -1;

    uint32_t m_vertexCount;
    uint32_t m_indexCount;

    void uploadData(MeshPart* data);
    std::vector<Mesh*> m_meshParts;

    static bool m_suppressDestroyMessage;
};

}; // namespace codex