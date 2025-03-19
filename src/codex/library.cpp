#include "codex/library.hpp"
#include "echo/console.hpp"

#include "imgui.h"
#include "lib/icons/IconsMaterialSymbols.h"

#include <unordered_map>

namespace Codex {

// Map enum values to string constants
static const std::unordered_map<FileType, const char*> FileTypeStrings = {
    {FileType::DIRECTORY , ICON_MS_FOLDER},
    {FileType::TEXT_FILE , ICON_MS_DESCRIPTION},
    {FileType::BINARY_FILE, ICON_MS_NOTE_STACK},
    {FileType::FONT_FILE, ICON_MS_FONT_DOWNLOAD},
    {FileType::IMAGE_FILE, ICON_MS_IMAGE},
    {FileType::AUDIO_FILE, ICON_MS_VOLUME_UP},
    {FileType::MESH_FILE, ICON_MS_DEPLOYED_CODE}
};

void Library::init() {
    m_assetsRoot = std::filesystem::current_path() / "assets";

    mapAssetsFolder();
    m_selectedNode = m_rootNode;
}

bool Library::formatPath(std::filesystem::path* path) const {
    // Check if the path is inside the assets root
    // (Disallow breaking out of the assets root)
    auto relative = std::filesystem::relative(*path, m_assetsRoot);
    auto correct = (!relative.empty() && !relative.string().contains(".."));
    auto exists = std::filesystem::exists(m_assetsRoot / relative);

    if (correct && exists) {
        // Copy relative path to original path
        *path = relative;
    } else {
        // Reset path to empty
        *path = "";
    }

    return correct && exists;
}

Library::Library() {}

Library::~Library() {}

void Library::mapAssetsFolder() {
    m_rootNode = new FileNode(m_assetsRoot, nullptr);
    m_fileLookupTable[m_rootNode->path] = std::unique_ptr<FileNode>(m_rootNode);

    for (auto& entry : std::filesystem::recursive_directory_iterator(m_assetsRoot)) {
        auto path = entry.path();
        if (path == m_assetsRoot) continue;

        auto parentPath = path.parent_path();
        formatPath(&parentPath);
        auto parent = m_fileLookupTable[parentPath].get();

        auto node = new FileNode(path, parent);
        m_fileLookupTable[node->path] = std::unique_ptr<FileNode>(node);
    }

    Echo::log("Assets folder mapped.");
}

void Library::loadNode(FileNode* node) {
    Library& instance = Library::instance();

    if (node == nullptr) {
        Echo::warn("Tried to load a null node.");
        return;
    }

    switch (node->type) {
    case FileType::IMAGE_FILE: {
        auto texture = instance.tryGetTexture(node);
        if (texture == nullptr) {
            texture = new Texture();
            texture->loadData(node);
            texture->loadResource();
            instance.m_textureLookupTable[node] = std::unique_ptr<Texture>(texture);
        }
        } break;
    default:
        Echo::warn("Unimplemented file type.");
        break;
    }
}

void Library::assetsWindow() {
    Library& instance = Library::instance();

    if (instance.m_selectedNode == nullptr) {
        return;
    }

    ImGui::Begin("Assets", nullptr);
    
    FileNode* folderNode[16]; // Magic limit
    folderNode[0] = instance.m_selectedNode;
    int folderIndex = 0;
    while (folderNode[folderIndex]->parent != nullptr) {
        folderNode[folderIndex + 1] = folderNode[folderIndex]->parent;
        folderIndex++;
    }

    for (int i = folderIndex; i >= 0; i--) {
        snprintf(instance.m_nameBuffer, 64, "%s%s##%p",
            ICON_MS_FOLDER_OPEN,
            i == folderIndex ? "." : folderNode[i]->name.c_str(),
            folderNode[i]
        );

        if (ImGui::Button(instance.m_nameBuffer, ImVec2(0, 32))) {
            instance.m_selectedNode = folderNode[i];
        }

        if (i != 0) {
            ImGui::SameLine();
        }
    }
    
    auto availableSpace = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("AssetsList", ImVec2(0, - availableSpace.x), true);

    int index = 1;
    for (auto& child : instance.m_selectedNode->children) {
        snprintf(instance.m_nameBuffer, 64, "%s%s##%p",
            FileTypeStrings.at(child->type),
            child->name.c_str(),
            child
        );

        if (ImGui::Selectable(instance.m_nameBuffer, index % 2 == 0)) {
            if (child->isDirectory) {
                instance.m_selectedNode = child;
            } else {
                instance.loadNode(child);
                instance.m_selectedTexture = instance.tryGetTexture(child);
            }
        }

        index++;
    }

    ImGui::EndChild();

    ImGui::BeginChild("AssetInspector", ImVec2(0, 0), true);
    ImGui::Text("Inspector");
    ImGui::Separator();

    if (instance.m_selectedTexture != nullptr) {
        ImGui::Text("Texture: %s", instance.m_selectedTexture->getNode()->name.c_str());
        
        auto image = instance.m_selectedTexture;

        auto availableSpace = ImGui::GetContentRegionAvail();
        struct{ int w, h; } imageSize = {image->getWidth(), image->getHeight()};

        if (imageSize.w > availableSpace.x || imageSize.h > availableSpace.y) {
            float scale = std::min(availableSpace.x / static_cast<float>(imageSize.w),
                                   availableSpace.y / static_cast<float>(imageSize.h));
            imageSize.w *= scale;
            imageSize.h *= scale;
        }


        ImGui::Image(image->getHandle(), ImVec2(imageSize.w, imageSize.h));
    }

    ImGui::EndChild();

    ImGui::End();
}

}