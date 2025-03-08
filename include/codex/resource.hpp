#pragma once

#include "codex/assetnode.hpp"

namespace Codex {

/**
 * @brief A resource that can be loaded from a file.
 */
class IResource {
friend class AssetNode;
friend class Assets;

public:
    IResource() : m_node(nullptr) {}
    IResource(AssetNode* node) : m_node(node) {}
    virtual ~IResource() {}

    /**
     * @return const AssetNode* The node it's attached to.
     * @attention May return `nullptr` if the resource was created during runtime.
     */
    const AssetNode* getAssetNode() const { return m_node; }
protected:
    AssetNode* m_node; // The node it's attached to
};

}; // namespace Codex