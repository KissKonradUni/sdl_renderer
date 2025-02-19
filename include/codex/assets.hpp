#pragma once

#include <string>
#include <vector>
#include <memory>

#include "texture.hpp"

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
protected:
    std::string m_name;
    std::string m_path;
    AssetType m_type;

    std::vector<std::shared_ptr<AssetNode>> m_children;
    std::shared_ptr<AssetNode> m_parent;
};

#define ASSETS_ROOT "./assets"
#define ASSET_ICONS "./assets/cinder/icons.png"

class Assets {
public:
    static Assets& instance() {
        static Assets instance;
        return instance;
    }

    Assets(const Assets&) = delete;
    Assets& operator=(const Assets&) = delete;

    void mapAssetsFolder();
    void printAssets();

    void assetsWindow();
protected:
    Assets() = default;
    ~Assets();

    std::unique_ptr<Texture> m_icons;

    std::shared_ptr<AssetNode> m_root;
    std::shared_ptr<AssetNode> m_currentNode;
    std::shared_ptr<AssetNode> m_selectedNode;

    void recursiveMap(std::shared_ptr<AssetNode> node);
    void recursivePrint(std::shared_ptr<AssetNode> node, int depth);
};

void assetsWindow();

}; // namespace Codex