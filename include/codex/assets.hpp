#pragma once

#include <functional>
#include <future>
#include <string>
#include <vector>
#include <memory>

#include "assetnode.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "shader.hpp"
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
    void setFunctions(ProcessFunc processor, LoaderFunc loader);
    void update();

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
    AssetNode* findAsset(const std::string& path);
    void printAssets();

    Scene& getCurrentScene() { return m_scene; }

    void assetsWindow();
    void previewWindow();

    AssetLibrary<ShaderResource, ShaderData>& getShaderLibrary() { return m_shaderLibrary; }
    AssetLibrary<Material, MaterialData>& getMaterialLibrary() { return m_materialLibrary; }
    AssetLibrary<Texture, TextureData>& getTextureLibrary() { return m_textureLibrary; }
    AssetLibrary<Mesh, SceneData>& getMeshLibrary() { return m_meshLibrary; }
protected:
    Assets();
    ~Assets();

    AssetNode* recursiveFindAsset(const std::string& path, AssetNode* node);

    AssetLibrary<ShaderResource, ShaderData> m_shaderLibrary;
    AssetLibrary<Material, MaterialData> m_materialLibrary;
    AssetLibrary<Texture, TextureData> m_textureLibrary;
    AssetLibrary<Mesh, SceneData> m_meshLibrary;

    // TODO: Make it be able to hold multiple scenes, and make them dynamically loadable
    Scene m_scene;

    bool m_iconsAvailable = true;
    std::shared_ptr<Texture> m_icons;

    std::shared_ptr<AssetNode> m_root;
    std::shared_ptr<AssetNode> m_currentNode;
    std::shared_ptr<AssetNode> m_selectedNode;

    std::shared_ptr<IResource> m_preview;
    AssetType m_previewType;

    void recursivePrint(std::shared_ptr<AssetNode> node, int depth);
    void recursiveMap(std::shared_ptr<AssetNode> node);
    void recursiveSort(std::shared_ptr<AssetNode> node);
};

}; // namespace Codex