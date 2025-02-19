#include "codex/assets.hpp"

#include "echo/console.hpp"
#include "echo/ui.hpp"
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

    Echo::log(indent + node->getName());

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

void Assets::assetsWindow() {
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
            }
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            
            m_selectedNode = child;
            if (m_selectedNode) {
                ImGui::Text("Asset info: ");
                ImGui::Text("- Name: %s", m_selectedNode->getName().c_str());
                ImGui::Text("- Type: %s", assetTypeToString(m_selectedNode->getType()).c_str());    
            }

            ImGui::EndTooltip();
        }
    }

    ImGui::EndChild();

    baseSize = ImGui::GetContentRegionAvail();

    ImGui::End();
}

void assetsWindow() {
    Codex::Assets::instance().assetsWindow();
}

}; // namespace Codex