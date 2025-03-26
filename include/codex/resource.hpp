#pragma once

#include "codex/filenode.hpp"

namespace codex {

/**
 * @brief Internal base class for easy pointer management
 */
class IResourceBase {
public:
    virtual ~IResourceBase() = default;

    virtual void loadData(const FileNode* file) = 0;
    virtual void loadResource() = 0;

    inline const bool isInitialized() const { return m_initialized; }
    inline const bool isRuntimeResource() const { return m_runtimeResource; }
    inline const FileNode* getNode() const { return m_node; }
protected:
    bool m_initialized = false;       // If the resource has been initialized
    bool m_runtimeResource = false;   // If the resource is runtime only (no node attached)
    const FileNode* m_node = nullptr; // The node in the library
};

/**
 * @brief Interface for resources (textures, models, etc.)
 *        that are most likely uploaded to the GPU
 * @tparam DataType The raw data struct of the resource
 */
template<typename DataType>
class IResource : public IResourceBase {
protected:
    std::unique_ptr<DataType> m_data; // The resource data (Freed after loading)
};

}; // namespace codex