#include "codex/material.hpp"
#include "codex/library.hpp"
#include "codex/texture.hpp"
#include "codex/shader.hpp"
#include "codex/mesh.hpp"

#include "cinder.hpp"

#include "imgui.h"

#include <IconsMaterialSymbols.h>
#include <unordered_map>

namespace codex {

// Map enum values to string constants
static const std::unordered_map<FileType, const char*> FileTypeStrings = {
    {FileType::DIRECTORY      , ICON_MS_FOLDER        },
    {FileType::SPECIAL        , ICON_MS_MANUFACTURING },
    
    {FileType::TEXT_FILE      , ICON_MS_DESCRIPTION   },
    {FileType::BINARY_FILE    , ICON_MS_NOTE_STACK    },

    {FileType::FONT_FILE      , ICON_MS_FONT_DOWNLOAD },
    {FileType::IMAGE_FILE     , ICON_MS_IMAGE         },
    {FileType::AUDIO_FILE     , ICON_MS_VOLUME_UP     },
    
    {FileType::MESH_FILE      , ICON_MS_DEPLOYED_CODE },
    {FileType::MESH_PART      , ICON_MS_QR_CODE       },

    {FileType::SHADER_FILE    , ICON_MS_STYLE         },
    {FileType::RAW_SHADER_FILE, ICON_MS_STROKE_PARTIAL},
    
    {FileType::MATERIAL_FILE  , ICON_MS_SHAPES        }
};

void Library::threadFunction() {
    while (this->m_asyncRunning) {
        m_asyncMutex.lock();
        if (m_asyncQueue.empty()) {
            m_asyncMutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Library::init() {
    m_assetsRoot = std::filesystem::current_path() / "assets";

    mapAssetsFolder();
    m_selectedNode = m_rootNode;

    std::filesystem::path runtimePath = m_assetsRoot / "runtime";
    formatPath(&runtimePath, true);
    m_runtimeNode = new FileNode(runtimePath, m_rootNode, true);
    m_runtimeNode->isDirectory = true;
    m_runtimeNode->path = runtimePath;
    m_runtimeNode->type = FileType::SPECIAL;
    m_fileLookupTable[m_runtimeNode->path] = std::unique_ptr<FileNode>(m_runtimeNode);

    cinder::log("Library initialized.");

    // Start the async IO thread
    m_asyncRunning = 1;
    m_asyncLoader = std::thread(&Library::threadFunction, this);

    cinder::log("Async IO thread started.");
}

bool Library::formatPath(std::filesystem::path* path, bool virt) const {
    // Check if the path is inside the assets root
    // (Disallow breaking out of the assets root)
    auto relative = std::filesystem::relative(*path, m_assetsRoot);
    auto correct = (!relative.empty() && !relative.string().contains(".."));
    auto exists = virt || std::filesystem::exists(m_assetsRoot / relative);

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

    cinder::log("Assets folder mapped.");
}

template<typename AssetType>
AssetType* Library::tryLoadResource(FileNode* node) {
    // Check if the node is valid
    if (node == nullptr) {
        cinder::warn("Tried loading invalid node.");
        return nullptr;
    }

    // Check if the node is already loaded
    if (m_resourceLookupTable.find(node) != m_resourceLookupTable.end()) {
        return static_cast<AssetType*>(m_resourceLookupTable[node].get());
    }

    // Create the action
    AsyncIOAction action;
    action.node = node;
    action.resource = new AssetType();
    m_resourceLookupTable[node] = std::unique_ptr<IResourceBase>(action.resource);

    if (action.resource == nullptr)
        return nullptr;

    // Add the action to the queue
    m_asyncMutex.lock();
    m_asyncQueue.push(action);
    m_asyncMutex.unlock();

    return static_cast<AssetType*>(action.resource);
}

FileNode* Library::requestRuntimeNode(const std::string& name, FileNode* parent) {
    auto parentNode = (parent == nullptr) ? m_runtimeNode : parent;
    
    std::filesystem::path runtimePath = parentNode->path / name;
    formatPath(&runtimePath, true);

    if (m_fileLookupTable.find(runtimePath) != m_fileLookupTable.end()) {
        return nullptr;
    }

    auto node = new FileNode(runtimePath, parentNode, true);
    m_fileLookupTable[runtimePath] = std::unique_ptr<FileNode>(node);

    return node;
}

void Library::registerRuntimeResource(IResourceBase* resource) {
    FileNode* node = const_cast<FileNode*>(resource->getNode());

    if (m_resourceLookupTable.find(node) != m_resourceLookupTable.end()) {
        cinder::warn("Tried registering a runtime resource to a used node.");
        return;
    }

    m_resourceLookupTable[node] = std::unique_ptr<IResourceBase>(resource);
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

void Library::assetsBrowser(Library* instance) {
    FileNode* folderNode[16]; // Magic limit
    folderNode[0] = instance->m_selectedNode;
    int folderIndex = 0;
    while (folderNode[folderIndex]->parent != nullptr) {
        folderNode[folderIndex + 1] = folderNode[folderIndex]->parent;
        folderIndex++;
    }

    for (int i = folderIndex; i >= 0; i--) {
        snprintf(instance->m_nameBuffer, 64, "%s%s##%p",
            ICON_MS_FOLDER_OPEN,
            i == folderIndex ? "." : folderNode[i]->name.c_str(),
            folderNode[i]
        );

        if (ImGui::Button(instance->m_nameBuffer, ImVec2(0, 32))) {
            instance->m_selectedNode = folderNode[i];
        }

        if (i != 0) {
            ImGui::SameLine();
        }
    }
    
    auto availableSpace = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("AssetsList", ImVec2(0, - availableSpace.x * 1.25f), ImGuiChildFlags_Borders);

    int index = 0;
    for (auto& child : instance->m_selectedNode->children) {
        snprintf(instance->m_nameBuffer, 64, "%s%s##%p",
            FileTypeStrings.at(child->type),
            child->name.c_str(),
            child
        );
        index++;

        if (!ImGui::Selectable(instance->m_nameBuffer, index % 2 == 0))
            continue;

        if (child->isDirectory) {
            instance->m_selectedNode = child;
            ImGui::EndChild();
            return;
        }

        instance->m_selectedType = child->type;

        #define TRY_LOAD(FileType, AssetType) \
            case FileType: \
                instance->m_selectedAsset = instance->tryLoadResource<AssetType>(child); \
                break;

        switch (child->type) {
            TRY_LOAD(FileType::IMAGE_FILE   , Texture )
            TRY_LOAD(FileType::SHADER_FILE  , Shader  )
            TRY_LOAD(FileType::MATERIAL_FILE, Material)
            TRY_LOAD(FileType::MESH_FILE    , Mesh    )
            default:
                instance->m_selectedAsset = nullptr;
                cinder::warn("Unsupported asset type.");
                break;
        }
    }

    ImGui::EndChild();
}

void Library::assetsInspector(Library* instance) {
    ImGui::BeginChild("AssetInspector", ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGui::Text("Inspector");
    ImGui::Separator();

    if (instance->m_selectedAsset == nullptr) {
        ImGui::Text("No/unsupported asset selected.");
        ImGui::EndChild();
        return;
    } else if (!instance->m_selectedAsset->isInitialized()) {
        ImGui::Text("Loading asset...");
        ImGui::EndChild();
        return;
    }

    ImGui::BeginChild("AssetInspectorPreview", ImVec2(0, -72));
    switch (instance->m_selectedType) {
        case FileType::IMAGE_FILE: {
            auto image = (Texture*)(instance->m_selectedAsset);

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
            } break;
        case FileType::SHADER_FILE: {
            auto shader = (Shader*)(instance->m_selectedAsset);

            ImGui::Text("Shader: %s", shader->getNode()->name.c_str());
            } break;
        case FileType::MATERIAL_FILE: {
            auto material = (Material*)(instance->m_selectedAsset);

            ImGui::Text("Material: %s", material->getNode()->name.c_str());
            ImGui::Text("Name: %s", material->getName().c_str());
            ImGui::Text("Textures: ");
            ImGui::BeginChild("MaterialTextures", ImVec2(0, 0), ImGuiChildFlags_Borders);
            auto textures = material->getTextures();
            int index = 0;
            for (const auto& [name, texture] : textures) {
                if (ImGui::Selectable(name.c_str(), index++ % 2 == 0)) {
                    instance->m_selectedAsset = texture;
                    instance->m_selectedType = FileType::IMAGE_FILE;
                }

                if (ImGui::IsItemHovered() && texture->isInitialized()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", texture->getNode()->path.c_str());
                    ImGui::EndTooltip();
                }
            }

            ImGui::EndChild();

            } break;
    default:
        ImGui::Text("No inspector for this asset type.");
        break;
    }
    ImGui::EndChild();

    if (ImGui::Selectable(ICON_MS_FOLDER "Go to containing folder", true)) {
        instance->m_selectedNode = instance->m_selectedAsset->getNode()->parent;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", instance->m_selectedAsset->getNode()->parent->path.c_str());
        ImGui::EndTooltip();
    }
    ImGui::Text("Runtime resource: %s", instance->m_selectedAsset->isRuntimeResource() ? ICON_MS_CHECK_CIRCLE : ICON_MS_CANCEL);

    ImGui::EndChild();
}

void Library::assetsWindow() {
    Library* instance = cinder::app->getLibrary();

    if (instance->m_selectedNode == nullptr) {
        return;
    }

    ImGui::Begin("Assets", nullptr);
    
    instance->assetsBrowser(instance);
    instance->assetsInspector(instance);

    ImGui::End();
}

}