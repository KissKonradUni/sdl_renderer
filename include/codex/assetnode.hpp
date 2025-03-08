#pragma once

#include <unordered_map>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Codex {

#define ASSET_TYPE_COUNT 8
enum AssetType : uint8_t {
    ASSET_FOLDER,
    ASSET_TEXTURE,
    ASSET_SHADER,
    ASSET_MODEL,
    ASSET_SOUND,
    ASSET_FONT,
    ASSET_TEXT_DATA,
    ASSET_BINARY_DATA
};

const std::unordered_map<std::string, AssetType> EXTENSION_MAP = {
    {".png", ASSET_TEXTURE},
    {".jpg", ASSET_TEXTURE},
    {".jpeg", ASSET_TEXTURE},
    {".bmp", ASSET_TEXTURE},

    {".material", ASSET_TEXT_DATA},

    {".wav", ASSET_SOUND},
    {".mp3", ASSET_SOUND},
    {".ogg", ASSET_SOUND},
    
    {".ttf", ASSET_FONT},
    
    {".json", ASSET_TEXT_DATA},
    {".xml", ASSET_TEXT_DATA},
    {".txt", ASSET_TEXT_DATA},
    {".csv", ASSET_TEXT_DATA},
    {".md", ASSET_TEXT_DATA},

    {".shader", ASSET_TEXT_DATA},

    {".bin", ASSET_BINARY_DATA},

    {".vert", ASSET_SHADER},
    {".frag", ASSET_SHADER},
    {".geom", ASSET_SHADER},
    {".glsl", ASSET_SHADER},

    {".glb", ASSET_MODEL},
    {".gltf", ASSET_MODEL},
    {".fbx", ASSET_MODEL},
    {".obj", ASSET_MODEL}
};

/**
 * @brief A node in the asset tree, holds the structure of the assets folder
 */
class AssetNode {
public:
    AssetNode(const std::string& path, AssetType type);
    ~AssetNode();

    const std::string& getName() const { return m_name; }
    const std::string& getPath() const { return m_path; }
    const AssetType getType() const { return m_type; }
    
    void addChild(std::shared_ptr<AssetNode> child) { m_children.push_back(child); }
    const std::vector<std::shared_ptr<AssetNode>>& getChildren() const { return m_children; }

    void setParent(std::shared_ptr<AssetNode> parent) { m_parent = parent; }
    std::shared_ptr<AssetNode> getParent() { return m_parent; }

    void sortChildren();
protected:
    std::string m_name;
    std::string m_path;
    AssetType m_type;

    std::vector<std::shared_ptr<AssetNode>> m_children;
    std::shared_ptr<AssetNode> m_parent;
};

}; // namespace Codex