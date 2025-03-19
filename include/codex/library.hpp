#pragma once

#include "codex/texture.hpp"
#include "filenode.hpp"

#include <filesystem>
#include <map>

namespace Codex {

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
    inline Texture* tryGetTexture(FileNode* node) const {
        try {
            return m_textureLookupTable.at(node).get();
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

    static void assetsWindow();
protected:
    std::filesystem::path m_assetsRoot;

    FileNode* m_rootNode;
    std::map<std::filesystem::path, std::unique_ptr<FileNode>> m_fileLookupTable;
    std::map<FileNode*, std::unique_ptr<Texture>> m_textureLookupTable;

    Library();
    ~Library();

    void mapAssetsFolder();
    void loadNode(FileNode* node);

    // UI Variables
    FileNode* m_selectedNode = nullptr;
    char m_nameBuffer[64];

    Texture* m_selectedTexture = nullptr;
};

}; // namespace Codex