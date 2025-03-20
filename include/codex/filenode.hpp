#pragma once

#include <unordered_map>

#include <filesystem>
#include <vector>

namespace Codex {

enum class FileType {
    DIRECTORY,
    TEXT_FILE,
    BINARY_FILE,

    FONT_FILE,
    IMAGE_FILE,
    AUDIO_FILE,
    MESH_FILE,
    
    SHADER_FILE,
    RAW_SHADER_FILE,

    MATERIAL_FILE,
};

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
    {".frag", FileType::RAW_SHADER_FILE},

    {".material", FileType::MATERIAL_FILE},
};

struct FileNode {
    FileNode* parent;
    std::vector<FileNode*> children;

    std::filesystem::path path; // For base path, lookup, etc.
    std::string name;           // For faster access
    std::string extension;      // For faster access
    FileType type;
    
    bool isDirectory;

    FileNode(std::filesystem::path& path, FileNode* parent);
    ~FileNode();
};

}; // namespace Codex