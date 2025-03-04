#include "codex/assets.hpp"

#include "echo/console.hpp"
#include "imgui.h"

#include <SDL3/SDL.h>
#include <filesystem>
#include <unordered_map>

namespace Codex {

const std::array<std::string, ASSET_TYPE_COUNT> ASSET_TYPE_STRINGS = {
    "Folder",
    "Texture",
    "Shader",
    "Model",
    "Sound",
    "Font",
    "Text Data",
    "Binary Data"
};

/* #region AssetLibrary<T,U> */
template<typename T, typename U>
AssetLibrary<T, U>::~AssetLibrary<T, U>() {
    m_assets.clear();
    m_loading.clear();
}

template<typename T, typename U>
AssetLibrary<T, U>::AssetPtr AssetLibrary<T, U>::get(const std::string& path) {
    auto it = m_assets.find(path);
    if (it != m_assets.end()) {
        return it->second;
    }
    
    // Check if already loading
    auto futureIt = m_loading.find(path);
    if (futureIt != m_loading.end()) {
        return m_processor(futureIt->second.get()); // Will block if not ready
    }

    // Start loading if we have a loader
    if (m_loader) {
        auto future = std::async(std::launch::async, m_loader, path);
        m_loading[path] = std::move(future);
        auto asset = m_loading[path].get();
        m_assets[path] = m_processor(asset);
        m_loading.erase(path);
        return m_assets[path];
    }

    return nullptr;
}

template<typename T, typename U>
AssetLibrary<T, U>::AssetPtr AssetLibrary<T, U>::tryGet(const std::string& path) {
    auto it = m_assets.find(path);
    if (it != m_assets.end()) {
        return it->second;
    }
    
    // Start loading if not already loading
    if (!isLoading(path) && m_loader) {
        auto future = std::async(std::launch::async, m_loader, path);
        m_loading[path] = std::move(future);
    }
    
    return nullptr;
}

template<typename T, typename U>
void AssetLibrary<T, U>::getAsync(const std::string& path, LoadCallback callback) {
    auto asset = tryGet(path);
    if (asset) {
        callback(asset);
        return;
    }

    m_callbacks[path].push_back(callback);
}

template<typename T, typename U>
void AssetLibrary<T, U>::update() {
    for (auto it = m_loading.begin(); it != m_loading.end();) {
        auto& [path, future] = *it;
        
        if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto asset = m_processor(future.get());
            m_assets[path] = asset;
            
            // Execute callbacks
            auto callbackIt = m_callbacks.find(path);
            if (callbackIt != m_callbacks.end()) {
                for (auto& callback : callbackIt->second) {
                    callback(asset);
                }
                m_callbacks.erase(callbackIt);
            }
            
            it = m_loading.erase(it);
        } else {
            ++it;
        }
    }
}

template<typename T, typename U>
void AssetLibrary<T, U>::setFunctions(ProcessFunc processor, LoaderFunc loader) {
    m_processor = processor;
    m_loader = loader;
}

template<typename T, typename U>
bool AssetLibrary<T, U>::isLoading(const std::string& path) const {
    return m_loading.find(path) != m_loading.end();
}

template<typename T, typename U>
void AssetLibrary<T, U>::preload(const std::string& path) {
    if (!isLoaded(path) && !isLoading(path) && m_loader) {
        m_loading[path] = std::async(std::launch::async, m_loader, path);
    }
}

template<typename T, typename U>
bool AssetLibrary<T,U>::isLoaded(const std::string& path) const {
    return m_assets.find(path) != m_assets.end();
}

template<typename T, typename U>
int AssetLibrary<T, U>::getAssetCount() const {
    return m_assets.size();
}

template<typename T, typename U>
int AssetLibrary<T, U>::getLoadingCount() const {
    return m_loading.size();
}
/* #endregion */

/* #region Assets */
Assets::Assets() {
    Echo::log("Asset manager starting up.");
    mapAssetsFolder();

    m_textureLibrary.setFunctions([](std::shared_ptr<TextureData> data) {
        return std::make_shared<Texture>(data.get());
    }, [](const std::string& path) {
        return Texture::loadTextureDataFromFile(path);
    });

    // TODO: Reimplement when new system is in place
    m_meshLibrary.setFunctions([](std::shared_ptr<SceneData> data) {
        //return Mesh::processCombinedSceneData(data);
        return nullptr;
    }, [](const std::string& path) {
        //return Mesh::loadSceneDataFromFile(path);
        return nullptr;
    });
}

Assets::~Assets() {
    Echo::log("Asset manager shutting down.");
    m_root.reset();
}

void Assets::mapAssetsFolder() {
    Echo::log("Mapping assets folder...");

    if (!(std::filesystem::exists(ASSETS_ROOT) && std::filesystem::is_directory(ASSETS_ROOT))) {
        Echo::warn("Assets folder does not exist. No assets will be available.");
        return;
    }

    m_root = std::make_shared<AssetNode>(ASSETS_ROOT, ASSET_FOLDER);
    m_currentNode = m_root;
    recursiveMap(m_root);
    recursiveSort(m_root);

    Echo::log("Assets folder mapped.");
    
    if (std::filesystem::exists(ASSET_ICONS)) {
        m_icons = Texture::loadTextureFromFile(ASSET_ICONS);
    } else {
        Echo::warn("Icons texture not found.");
    }
}

void Assets::printAssets() {
    Echo::log("Assets: ");

    if (m_root) {
        recursivePrint(m_root, 0);
    } else {
        Echo::warn("No assets have been mapped.");
    }
}

void Assets::recursivePrint(std::shared_ptr<AssetNode> node, int depth) {
    std::string indent = "";
    for (int i = 0; i < depth; i++) {
        indent += "  ";
    }

    if (node->getType() == ASSET_FOLDER) {
        Echo::log(indent + "\\ " + node->getName());
    } else {
        Echo::log(indent + "| " + node->getName());
    }

    for (const auto& child : node->getChildren()) {
        recursivePrint(child, depth + 1);
    }
}

void Assets::recursiveMap(std::shared_ptr<AssetNode> node) {
    for (const auto& entry : std::filesystem::directory_iterator(node->getPath())) {
        auto path = entry.path().string();
        if (entry.is_directory()) {
            auto child = std::make_shared<AssetNode>(path, ASSET_FOLDER);
            child->setParent(node);
            node->addChild(child);
            recursiveMap(child);
        } else {
            auto extension = entry.path().extension().string();
            auto type = EXTENSION_MAP.find(extension);
            if (type != EXTENSION_MAP.end()) {
                auto child = std::make_shared<AssetNode>(path, type->second);
                child->setParent(node);
                node->addChild(child);
            } else {
                Echo::warn(std::string("Unknown asset type for file: ") + path);
            }
        }
    }
}

void Assets::recursiveSort(std::shared_ptr<AssetNode> node) {
    if (node->getType() == ASSET_FOLDER) {
        node->sortChildren();
    }

    for (const auto& child : node->getChildren()) {
        recursiveSort(child);
    }
}

void Assets::assetsWindow() {
    ImGui::Begin("Asset browser", nullptr);

    m_textureLibrary.update();
    m_meshLibrary.update();

    if (!m_currentNode) {
        ImGui::Text("No assets have been mapped.");
        ImGui::End();
        return;
    }

    if (ImGui::Button("<", ImVec2(32, 32))) {
        if (m_currentNode->getParent()) {
            m_currentNode = m_currentNode->getParent();
        }
    }
    ImGui::SameLine();
    ImGui::Text("%s", m_currentNode->getPath().c_str());

    auto baseSize = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("##assets", ImVec2(baseSize.x, baseSize.y), ImGuiChildFlags_Borders);
    
    baseSize = ImGui::GetContentRegionAvail();
    for (const auto& child : m_currentNode->getChildren()) {
        if (m_icons) {
            float iconIndex = child->getType();
            ImGui::Image(m_icons->getHandle(), ImVec2(48, 48), ImVec2(iconIndex / ASSET_TYPE_COUNT, 1), ImVec2((iconIndex + 1) / ASSET_TYPE_COUNT, 0));
            ImGui::SameLine();
        }
        if (ImGui::Button(child->getName().c_str(), ImVec2(baseSize.x - 56, 48))) {
            if (child->getType() == ASSET_FOLDER) {
                m_currentNode = child;
                m_selectedNode = nullptr;
            } else {
                m_selectedNode = child;

                // Test, TODO: Expand upon
                if (child->getType() == ASSET_TEXTURE) {
                    m_textureLibrary.getAsync(child->getPath(), [this, child](std::shared_ptr<Texture> texture) {
                        m_texturePreview = texture;
                        texture->setAssetNode(child.get());
                    });
                }
            }
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            
            ImGui::Text("Asset info: ");
            ImGui::Text("- Name: %s", child->getName().c_str());
            ImGui::Text("- Type: %s", ASSET_TYPE_STRINGS[child->getType()].c_str());    

            ImGui::EndTooltip();
        }
    }

    ImGui::EndChild();

    baseSize = ImGui::GetContentRegionAvail();

    ImGui::End();
}

void Assets::previewWindow() {
    ImGui::Begin("Asset viewer", nullptr);

    static int activeTab = 0;
    ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None);
    if (ImGui::TabItemButton("Preview")) {
        activeTab = 0;
    }
    if (ImGui::TabItemButton("Stats")) {
        activeTab = 1;
    }

    if (activeTab == 0) {
        if (m_texturePreview) {
            ImGui::Text("%dx%d:%d", m_texturePreview->getWidth(), m_texturePreview->getHeight(), m_texturePreview->getChannels());
        } else {
            ImGui::Text("No texture selected.");
        }

        ImGui::Separator();

        auto area = ImGui::GetContentRegionAvail();

        if (m_texturePreview) {
            float imageAspect = (float)m_texturePreview->getWidth() / (float)m_texturePreview->getHeight();
            float windowAspect = area.x / area.y;
            float width, height;
            
            if (imageAspect > windowAspect) {
                width = area.x;
                height = area.x / imageAspect;
            } else {
                width = area.y * imageAspect;
                height = area.y;
            }

            ImGui::Image(m_texturePreview->getHandle(), ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
        }
    } else if (activeTab == 1) {
        ImGui::Text("Asset count: ");
        ImGui::Text("- Textures: %d", m_textureLibrary.getAssetCount());
        ImGui::Text("- Meshes: %d", m_meshLibrary.getAssetCount());

        ImGui::Separator();

        ImGui::Text("Currently loading: ");
        ImGui::Text("- Textures: %d", m_textureLibrary.getLoadingCount());
        ImGui::Text("- Meshes: %d", m_meshLibrary.getLoadingCount());
    } 
    ImGui::EndTabBar();
    ImGui::End();
}

/* #endregion */

// Explicit template instantiations
template class AssetLibrary<Texture, TextureData>;
template class AssetLibrary<Mesh, SceneData>;

}; // namespace Codex