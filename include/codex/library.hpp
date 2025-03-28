#pragma once

#include "codex/shader.hpp"

#include "resource.hpp"
#include "filenode.hpp"

#include <filesystem>
#include <thread>
#include <queue>
#include <map>

// TODO: remove
extern Codex::Shader* shader;

namespace Codex {

struct AsyncIOAction {
    FileNode* node;
    IResourceBase* resource;
};

class Library {
friend FileNode;
public:
    static Library& instance() {
        static Library instance;
        return instance;
    }

    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;

    /**
     * @brief Initializes the asset library
     */
    void init();

    /**
     * @return constexpr const std::filesystem::path& 
     *         The root path of the assets directory
     */
    inline constexpr const std::filesystem::path& getAssetsRoot() const {
        return m_assetsRoot;
    }

    /**
     * @brief Gets a node from the asset library
     * @attention Excepts formatted paths
     * 
     * @param path The path to the asset
     * @return FileNode* The node (nullptr if not found)
     */
    inline FileNode* tryGetAssetNode(const std::filesystem::path& path) const {
        try {
            return m_fileLookupTable.at(path).get();
        } catch (const std::out_of_range& e) {
            return nullptr;
        }
    }

    /**
     * @brief Gets a resource from the asset library
     * 
     * @tparam AssetType The type of the asset (Texture, Shader, etc.)
     * @param node The file node of the asset
     * @return AssetType* The asset (nullptr if not found)
     */
    template<typename AssetType = IResourceBase>
    inline AssetType* tryGetLoadedResource(FileNode* node) const {
        if (m_resourceLookupTable.find(node) != m_resourceLookupTable.end()) {
            return static_cast<AssetType*>(m_resourceLookupTable.at(node).get());
        }
        return nullptr;
    }

    /**
     * @brief Loads a resource from the asset library
     *        If the resource is already loaded, returns the existing one
     * 
     * @tparam AssetType The type of the asset (Texture, Shader, etc.)
     * @param node The file node of the asset
     * @return AssetType* The asset (nullptr if not found)
     */
    template<typename AssetType>
    AssetType* tryLoadResource(FileNode* node);

    /**
     * @brief Requests a runtime node for temporary assets.
     *        This is used for creating runtime resources.
     *        For accessing runtime resources, use @see tryGetAssetNode and @see tryGetLoadedResource
     * 
     * @param name The name of the runtime node
     * @param parent The parent node (optional)
     * @return FileNode* The runtime node (nullptr if already exists)
     */
    FileNode* requestRuntimeNode(const std::string& name, FileNode* parent = nullptr);

    /**
     * @brief Registers a runtime resource.
     * 
     * @param resource The runtime resource
     */
    void registerRuntimeResource(IResourceBase* resource);

    /**
     * @brief Formats a path to be relative to the assets root.
     * Checks if the path is inside the assets root.
     * Also checks if the path exists.
     * 
     * @param path The path to format
     * @param virt If the path is virtual (does not exist) [default: false]
     * @return true The path is valid
     * @return false The path does not exist or is outside the assets root
     */
    bool formatPath(std::filesystem::path* path, bool virt = false) const;

    /**
     * @brief Checks for finished asynchronous actions.
     *        Should be called every once in a while.
     */
    void checkForFinishedAsync();

    /**
     * @brief The assets window for the UI
     */
    static void assetsWindow();
protected:
    // Tree structure of the assets folder
    std::filesystem::path m_assetsRoot;
    FileNode* m_rootNode;
    FileNode* m_runtimeNode;

    // Lookup tables for quick access
    std::map<std::filesystem::path, std::unique_ptr<FileNode>> m_fileLookupTable;
    std::map<FileNode*, std::unique_ptr<IResourceBase>> m_resourceLookupTable;

    std::queue<AsyncIOAction> m_asyncQueue;
    std::queue<AsyncIOAction> m_asyncFinished;
    volatile int m_asyncRunning = 0;
    volatile int m_asyncCheck = 0;
    std::thread m_asyncLoader;
    std::mutex m_asyncMutex;

    // Internal functions
    Library();
    ~Library();

    void threadFunction();
    void mapAssetsFolder();

    // UI
    void assetsBrowser(Library& instance);
    void assetsInspector(Library& instance);

    FileNode* m_selectedNode = nullptr;
    char m_nameBuffer[64];

    FileType m_selectedType = FileType::DIRECTORY;
    IResourceBase* m_selectedAsset = nullptr;
};

}; // namespace Codex