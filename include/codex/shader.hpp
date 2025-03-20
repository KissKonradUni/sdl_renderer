#pragma once

#include "codex/resource.hpp"
#include "floatmath.hpp"

#include <SDL3/SDL.h>

#include <string>
#include <unordered_map>

namespace Codex {

/**
 * @brief Struct type for uniform buffers.
 * Any uniform buffer data type should be extended from this struct.
 * While templating would've been an option, this forces the data to be aligned to 16 bytes.
 */
struct alignas(16) UniformBufferData {};

/**
 * @brief Uniform buffer class.
 * Allow a direct interface to OpenGL uniform buffers.
 * Using this there's no need to manually name and set uniform locations.
 * The data update is done by the user, at once.
 */
class UniformBuffer {
public:
    UniformBuffer(size_t size, int binding);

    /**
     * @brief Binds the uniform buffer. Required before updating data.
     */
    void bind();
    /**
     * @brief Updates the data in the uniform buffer.
     * Automatically binds the buffer.
     * @param data The data to update the buffer with.
     */
    void updateData(const UniformBufferData* data);
protected:
    unsigned int m_uniformBufferObjectHandle;
    size_t m_size;
};

/**
 * @brief Struct for holding raw shader source code.
 */
struct ShaderData {
public:
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
};

/**
 * @brief The OpenGL shader encapsulating class.
 * Allows for easy shader creation and management.
 * Manages and caches uniform locations.
 */
class Shader : public IResource<ShaderData> {
public:
    Shader();
    virtual ~Shader();

    void bind();
    
    void setUniform(const std::string& name, const int& value);
    void setUniform(const std::string& name, const vector4f& value);
    void setUniform(const std::string& name, const matrix4x4f& value);

    void loadData(const FileNode* file);
    void loadResource();
protected:
    unsigned int m_programHandle;
    std::unordered_map<std::string, unsigned int> m_uniformLocations;

    unsigned int getUniformLocation(const std::string& name);
};

}; // namespace Codex