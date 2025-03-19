#pragma once

#include "codex/texture.hpp"
#include "codex/shader.hpp"
#include "filenode.hpp"

#include <filesystem>
#include <thread>
#include <queue>
#include <map>

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
     * @brief Gets a texture from the asset library
     * @attention Excepts formatted paths
     *
     * @param node The node to get the texture from
     * @return Texture* The texture (nullptr if not found)
     */
    inline Texture* tryGetLoadedTexture(FileNode* node) const {
        try {
            return m_textureLookupTable.at(node).get();
        } catch (const std::out_of_range& e) {
            return nullptr;
        }
    }

    /**
     * @brief Gets a shader from the asset library
     * @attention Excepts formatted paths

     * @param node The node to get the shader from
     * @return Shader* The shader (nullptr if not found)
     */
    inline Shader* tryGetLoadedShader(FileNode* node) const {
        try {
            return m_shaderLookupTable.at(node).get();
        } catch (const std::out_of_range& e) {
            return nullptr;
        }
    }
    
    /**
     * @brief Formats a path to be relative to the assets root.
     * Checks if the path is inside the assets root.
     * Also checks if the path exists.
     * 
     * @param path The path to format
     * @return true The path is valid
     * @return false The path does not exist or is outside the assets root
     */
    bool formatPath(std::filesystem::path* path) const;

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

    // Lookup tables for quick access
    std::map<std::filesystem::path, std::unique_ptr<FileNode>> m_fileLookupTable;
    std::map<FileNode*, std::unique_ptr<Texture>> m_textureLookupTable;
    std::map<FileNode*, std::unique_ptr<Shader>> m_shaderLookupTable;

    // Async loading
    IResourceBase* asnycLoadResource(FileNode* node);

    std::queue<AsyncIOAction> m_asyncQueue;
    std::queue<AsyncIOAction> m_asyncFinished;
    volatile int m_asyncRunning = 0;
    volatile int m_asyncCheck = 0;
    std::thread m_asyncLoader;
    std::mutex m_asyncMutex;

    // Internal functions
    Library();
    ~Library();

    void mapAssetsFolder();

    // UI Variables
    FileNode* m_selectedNode = nullptr;
    char m_nameBuffer[64];

    FileType m_selectedType = FileType::DIRECTORY;
    IResourceBase* m_selectedAsset = nullptr;
};

}; // namespace Codex