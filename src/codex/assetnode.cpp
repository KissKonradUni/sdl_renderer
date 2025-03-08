#include "codex/assetnode.hpp"

#include <filesystem>

namespace Codex {

/* #region AssetNode */
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
/* #endregion */

} // namespace Codex