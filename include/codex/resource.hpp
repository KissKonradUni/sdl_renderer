#pragma once

#include "codex/filenode.hpp"

namespace Codex {

template<typename DataType>
class IResource {
public:
    virtual void loadData(const FileNode* file) = 0;
    virtual void loadResource() = 0;

    inline const bool isInitialized() const { return m_initialized; }
    inline const bool isRuntimeResource() const { return m_runtimeResource; }
    inline const FileNode* getNode() const { return m_node; }
protected:
    bool m_initialized = false;       // If the resource has been initialized
    bool m_runtimeResource = false;   // If the resource is runtime only (no node attached)
    const FileNode* m_node;           // The node in the library
    std::unique_ptr<DataType> m_data; // The resource data (Freed after loading)
};

}; // namespace Codex