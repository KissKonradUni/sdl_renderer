#pragma once

#include <functional>
#include <future>
#include <string>
#include <vector>
#include <memory>

#include "assetnode.hpp"
#include "texture.hpp"
#include "scene.hpp"
#include "mesh.hpp"

namespace Codex {

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
    void update();
    void setFunctions(ProcessFunc processor, LoaderFunc loader);

    bool isLoading(const std::string& path) const;
    bool isLoaded(const std::string& path) const;
    void preload(const std::string& path);

    int getAssetCount() const;
    int getLoadingCount() const;
private:
    std::unordered_map<std::string, AssetPtr> m_assets;
    std::unordered_map<std::string, std::future<DataPtr>> m_loading;
    std::unordered_map<std::string, std::vector<LoadCallback>> m_callbacks;
    ProcessFunc m_processor;
    LoaderFunc m_loader;
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
    AssetLibrary<Mesh, SceneData>& getMeshLibrary() { return m_meshLibrary; }
protected:
    Assets();
    ~Assets();

    AssetLibrary<Texture, TextureData> m_textureLibrary;
    AssetLibrary<Mesh, SceneData> m_meshLibrary;

    // TODO: Make it be able to hold multiple scenes, and make them dynamically loadable
    Scene m_scene;

    std::shared_ptr<Texture> m_icons;

    std::shared_ptr<AssetNode> m_root;
    std::shared_ptr<AssetNode> m_currentNode;
    std::shared_ptr<AssetNode> m_selectedNode;

    std::shared_ptr<Texture> m_texturePreview;

    void recursivePrint(std::shared_ptr<AssetNode> node, int depth);
    void recursiveMap(std::shared_ptr<AssetNode> node);
    void recursiveSort(std::shared_ptr<AssetNode> node);
};

}; // namespace Codex