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

    Echo::log("Library initialized.");

    // Start the async IO thread
    m_asyncRunning = 1;
    m_asyncLoader = std::thread([this]() {
        while (this->m_asyncRunning) {
            m_asyncMutex.lock();
            if (!m_asyncQueue.empty()) {
                auto action = m_asyncQueue.front();
                m_asyncQueue.pop();
                m_asyncMutex.unlock();

                action.resource->loadData(action.node);
                
                m_asyncMutex.lock();
                m_asyncFinished.push(action);

                // Volatile used as functional access
                auto asyncCheck = m_asyncCheck;
                asyncCheck++;
                m_asyncCheck = asyncCheck;

                m_asyncMutex.unlock();
            } else {
                m_asyncMutex.unlock();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    Echo::log("Async IO thread started.");
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

Library::~Library() {
    m_asyncRunning = 0;
    m_asyncLoader.join();
    Echo::log("Library destroyed.");
}

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

IResourceBase* Library::asnycLoadResource(FileNode* node) {
    // Check if the node is valid
    if (node == nullptr) {
        Echo::warn("Tried loading invalid node.");
        return nullptr;
    }

    // Check if the node is already loaded
    if (m_textureLookupTable.find(node) != m_textureLookupTable.end()) {
        return m_textureLookupTable[node].get();
    }
    // TODO: Add other lookup tables

    // Create the action
    AsyncIOAction action;
    action.node = node;
    switch (node->type) {
    case FileType::IMAGE_FILE: {
        auto texture = new Texture();
        action.resource = texture;
        m_textureLookupTable[node] = std::unique_ptr<Texture>(texture);
    } break;
    default:
        // TODO: Add other file types
        Echo::warn("Unimplemented file type.");
        return nullptr;
    }

    // Add the action to the queue
    m_asyncMutex.lock();
    m_asyncQueue.push(action);
    m_asyncMutex.unlock();

    return action.resource;
}

void Library::checkForFinishedAsync() {
    if (m_asyncCheck == 0) {
        return;
    }
    
    m_asyncMutex.lock();
    while (!m_asyncFinished.empty()) {
        auto action = m_asyncFinished.front();
        m_asyncFinished.pop();

        // Volatile used as functional access
        auto asyncCheck = m_asyncCheck;
        asyncCheck--;
        m_asyncCheck = asyncCheck;

        action.resource->loadResource();
    }
    m_asyncMutex.unlock();
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
                instance.asnycLoadResource(child);
                instance.m_selectedTexture = instance.tryGetLoadedTexture(child);
            }
        }

        index++;
    }

    ImGui::EndChild();

    ImGui::BeginChild("AssetInspector", ImVec2(0, 0), true);
    ImGui::Text("Inspector");
    ImGui::Separator();

    if (instance.m_selectedTexture != nullptr) {        
        auto image = instance.m_selectedTexture;
        if (image->isInitialized()) {
            ImGui::Text("Texture: %s", image->getNode()->name.c_str());

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
    }

    ImGui::EndChild();

    ImGui::End();
}

}