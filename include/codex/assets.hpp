#pragma once

#include <functional>
#include <future>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <list>

#include "texture.hpp"
#include "scene.hpp"

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

template<typename T, typename U>
class AssetLibrary {
public:
    using DataPtr = std::shared_ptr<U>;
    using AssetPtr = std::shared_ptr<T>;
    using LoaderFunc = std::function<DataPtr(const std::string&)>;
    using ProcessFunc = std::function<AssetPtr(DataPtr)>;
    using LoadCallback = std::function<void(AssetPtr)>;

    AssetLibrary<T,U>() = default;
    ~AssetLibrary<T,U>();
    AssetPtr get(const std::string& path);
    AssetPtr tryGet(const std::string& path);
    void getAsync(const std::string& path, LoadCallback callback);
    void getAsync(const std::string& path, AssetPtr out);
    void update();
    void setFunctions(ProcessFunc processor, LoaderFunc loader);

    bool isLoading(const std::string& path) const;
    void preload(const std::string& path) const;
    bool isLoaded(const std::string& path) const;
private:
    std::unordered_map<std::string, AssetPtr> m_assets;
    std::unordered_map<std::string, std::future<DataPtr>> m_loading;
    std::unordered_map<std::string, std::vector<LoadCallback>> m_callbacks;
    ProcessFunc m_processor;
    LoaderFunc m_loader;
};

template <typename T>
class Offloader {
public:
    Offloader(std::function<void(std::shared_ptr<T>)> callback);
    ~Offloader();

    void update();
    void run(std::function<std::shared_ptr<T>()> threadFunc);
protected:
    std::function<void(std::shared_ptr<T>)> m_callback = nullptr;

    std::list<std::function<std::shared_ptr<T>()>> m_threadQueue;
    std::function<std::shared_ptr<T>()> m_threadFunc = nullptr;
    std::atomic<bool> m_threadRunning = true;
    std::thread m_thread;

    std::shared_ptr<T> m_resultBuffer = nullptr;
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

    Scene& getCurrentScene() { return m_scene; }

    void assetsWindow();
    void previewWindow();

    AssetLibrary<Texture, TextureData>& getTextureLibrary() { return m_textureLibrary; }
protected:
    Assets();
    ~Assets();

    AssetLibrary<Texture, TextureData> m_textureLibrary;

    // TODO: Make it be able to hold multiple scenes, and make them dynamically loadable
    Scene m_scene;

    std::shared_ptr<Texture> m_icons;

    std::shared_ptr<AssetNode> m_root;
    std::shared_ptr<AssetNode> m_currentNode;
    std::shared_ptr<AssetNode> m_selectedNode;

    void recursivePrint(std::shared_ptr<AssetNode> node, int depth);
    void recursiveMap(std::shared_ptr<AssetNode> node);
    void recursiveSort(std::shared_ptr<AssetNode> node);
};

}; // namespace Codex