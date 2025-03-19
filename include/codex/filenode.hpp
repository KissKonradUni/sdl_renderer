#pragma once

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
    RAW_SHADER_FILE
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