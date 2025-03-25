#include "codex/filenode.hpp"
#include "codex/library.hpp"

namespace Codex {

FileNode::FileNode(std::filesystem::path& path, FileNode* parent, bool virt) {
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

    if (!virt)
        Library::instance().formatPath(&this->path);
}

FileNode::~FileNode() {
    // We don't delete children, as they are owned by the library
}

}; // namespace Codex