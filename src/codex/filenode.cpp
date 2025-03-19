#include "codex/filenode.hpp"
#include "codex/library.hpp"

#include <unordered_map>

namespace Codex {

// Map string extensions to enum values
static const std::unordered_map<std::string, FileType> ExtensionIcons = {
    {".txt", FileType::TEXT_FILE},
    {".md", FileType::TEXT_FILE},

    {".bin", FileType::BINARY_FILE},

    {".ttf", FileType::FONT_FILE},
    {".otf", FileType::FONT_FILE},

    {".png", FileType::IMAGE_FILE},
    {".jpg", FileType::IMAGE_FILE},
    {".jpeg", FileType::IMAGE_FILE},
    {".bmp", FileType::IMAGE_FILE},
    
    {".wav", FileType::AUDIO_FILE},
    {".mp3", FileType::AUDIO_FILE},
    {".ogg", FileType::AUDIO_FILE},

    {".glb", FileType::MESH_FILE},
    {".gltf", FileType::MESH_FILE},

    {".shader", FileType::SHADER_FILE},
    {".vert", FileType::RAW_SHADER_FILE},
    {".frag", FileType::RAW_SHADER_FILE}
};

FileNode::FileNode(std::filesystem::path& path, FileNode* parent) {
    if (parent != nullptr) {    
        this->parent = parent;
        this->parent->children.push_back(this);
    } else {
        this->parent = nullptr;
    }
    this->path = path;
    this->name = path.filename().string();
    this->extension = path.extension().string();
    this->isDirectory = std::filesystem::is_directory(path);
    if (this->isDirectory) {
        this->type = FileType::DIRECTORY;
    } else {
        try {
            this->type = ExtensionIcons.at(this->extension);
        } catch (const std::out_of_range& e) {
            this->type = FileType::TEXT_FILE;
        }
    }
    Library::instance().formatPath(&this->path);
}

FileNode::~FileNode() {
    // We don't delete children, as they are owned by the library
}

}; // namespace Codex