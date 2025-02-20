#include "codex/assets.hpp"

#include "echo/console.hpp"
#include "imgui.h"

#include <SDL3/SDL.h>
#include <filesystem>
#include <unordered_map>

namespace Codex {

const std::string assetTypeToString(const AssetType type) {
    switch (type) {
        case ASSET_FOLDER: return "Folder";
        case ASSET_TEXTURE: return "Texture";
        case ASSET_SHADER: return "Shader";
        case ASSET_MODEL: return "Model";
        case ASSET_SOUND: return "Sound";
        case ASSET_FONT: return "Font";
        case ASSET_TEXT_DATA: return "Text Data";
        case ASSET_BINARY_DATA: return "Binary Data";
        default: return "Unknown";
    }
}

const std::unordered_map<std::string, AssetType> EXTENSION_MAP = {
    {".png", ASSET_TEXTURE},
    {".jpg", ASSET_TEXTURE},
    {".jpeg", ASSET_TEXTURE},
    {".bmp", ASSET_TEXTURE},

    {".wav", ASSET_SOUND},
    {".mp3", ASSET_SOUND},
    {".ogg", ASSET_SOUND},
    
    {".ttf", ASSET_FONT},
    
    {".json", ASSET_TEXT_DATA},
    {".xml", ASSET_TEXT_DATA},
    {".txt", ASSET_TEXT_DATA},
    {".csv", ASSET_TEXT_DATA},
    {".md", ASSET_TEXT_DATA},

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

AssetNode::AssetNode(const std::string& path, AssetType type) 
    : m_path(path), m_type(type) 
{
    m_name = std::filesystem::path(path).filename().string();
}

AssetNode::~AssetNode() {
    m_children.clear();
}

void AssetNode::sortChildren() {
    std::sort(this->m_children.begin(), this->m_children.end(), [](const auto& a, const auto& b) {
        if (a->getType() == ASSET_FOLDER && b->getType() != ASSET_FOLDER) {
            return true;
        } else if (a->getType() != ASSET_FOLDER && b->getType() == ASSET_FOLDER) {
            return false;
        } else {
            return a->getName() < b->getName();
        }
    });
}

LazyLoaderBase::LazyLoaderBase() : m_state(UNINITIALIZED) {}
void LazyLoaderBase::update() {}

std::list<LazyLoaderBase*> m_loaders;

template <typename T>
LazyLoader<T>::LazyLoader(
    std::function<std::shared_ptr<T>(const std::string&)> loader,
    std::function<void(std::shared_ptr<T>&)> callback
) : m_loader(loader), m_callback(callback) {
    Echo::log("New lazy loader thread created.");

    m_loaders.push_back(this);
}

template <typename T>
LazyLoader<T>::~LazyLoader() {
    if (m_thread.joinable()) {
        m_thread.join();
    }

    m_loaders.erase(std::remove(m_loaders.begin(), m_loaders.end(), this), m_loaders.end());

    Echo::log("Lazy loader finished.");
}

template <typename T>
void LazyLoader<T>::load(const std::string& path) {
    m_path = path;
    m_state = LOADING;
    m_thread = std::thread([this]() {
        while (m_state != STOPPED) {
            if (m_state == LOADING) {
                m_data = m_loader(m_path);
                m_state = DONE;
            }
        }
    });
}

template <typename T>
void LazyLoader<T>::update() {
    if (m_state == DONE) {
        if (m_thread.joinable()) {
            m_state = STOPPED;
            m_thread.join();
            m_callback(m_data);
            m_state = UNINITIALIZED;
        }
    }
}

void updateAllLoaders() {
    for (const auto& loader : m_loaders) {
        loader->update();
    }
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

// TODO: Remove this
std::shared_ptr<Texture> lastLoadedTexture = nullptr;
std::shared_ptr<LazyLoader<TextureData>> textureLoader = nullptr;

void Assets::assetsWindow() {
    updateAllLoaders();

    ImGui::Begin("Assets", nullptr);

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

    auto baseSize  = ImGui::GetContentRegionAvail();

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
                    textureLoader = std::make_shared<LazyLoader<TextureData>>(Texture::loadTextureDataFromFile, [](std::shared_ptr<TextureData>& texture) {
                        Echo::log("Texture loaded.");
                        lastLoadedTexture = std::make_shared<Texture>(texture->data, texture->width, texture->height, texture->channels);
                    });
                    textureLoader->load(child->getPath());
                }
            }
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            
            ImGui::Text("Asset info: ");
            ImGui::Text("- Name: %s", child->getName().c_str());
            ImGui::Text("- Type: %s", assetTypeToString(child->getType()).c_str());    

            ImGui::EndTooltip();
        }
    }

    ImGui::EndChild();

    baseSize = ImGui::GetContentRegionAvail();

    ImGui::End();
}

void Assets::previewWindow() {
    ImGui::Begin("Preview", nullptr);

    if (textureLoader)
        switch (textureLoader->getState()) {
            case LOADING:
                ImGui::Text("Loading texture...");
                break;
            default:
                ImGui::Text("%dx%d:%d", lastLoadedTexture->getWidth(), lastLoadedTexture->getHeight(), lastLoadedTexture->getChannels());
                break;
        }

    auto area = ImGui::GetContentRegionAvail();

    if (lastLoadedTexture) {
        float imageAspect = (float)lastLoadedTexture->getWidth() / (float)lastLoadedTexture->getHeight();
        float windowAspect = area.x / area.y;
        float width, height;
        
        if (imageAspect > windowAspect) {
            width = area.x;
            height = area.x / imageAspect;
        } else {
            width = area.y * imageAspect;
            height = area.y;
        }

        ImGui::Image(lastLoadedTexture->getHandle(), ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
    }

    ImGui::End();
}

void assetsWindow() {
    Codex::Assets::instance().assetsWindow();
}

}; // namespace Codex