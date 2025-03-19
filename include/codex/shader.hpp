#pragma once

#include "floatmath.hpp"

#include <SDL3/SDL.h>

#include <memory>
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

// Forward declaration
class Shader;

/**
 * @brief A wrapper to hold the shader in a resource managed way.
 * This allows the usage of additional metadata and also allows better querying.
 * @attention This class is not meant to be used directly.
 * @attention This class does not create the shader, it only holds it.
 * @ref Assets
 */
class ShaderResource {
friend class Assets;
public:
    ShaderResource(const std::string& path);
    ~ShaderResource();

    /**
     * @brief Gets the shader program of the resource.
     * @return Shader* - The shader program.
     */
    Shader* getShader() { return m_program.get(); }
protected:
    std::unique_ptr<Shader> m_program;

    std::string m_name;
    std::string m_vertexShaderFilename;
    std::string m_fragmentShaderFilename;
};

/**
 * @brief Struct for holding raw shader source code.
 */
struct ShaderData {
public:
    std::string vertexShaderSource;
    std::string fragmentShaderSource;

    std::shared_ptr<ShaderResource> resource;
};

/**
 * @brief The OpenGL shader encapsulating class.
 * Allows for easy shader creation and management.
 * Manages and caches uniform locations.
 */
class Shader {
public:
    Shader(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
    ~Shader();

    void bind();
    
    void setUniform(const std::string& name, const int& value);
    void setUniform(const std::string& name, const vector4f& value);
    void setUniform(const std::string& name, const matrix4x4f& value);

    static std::shared_ptr<ShaderData> loadShaderDataFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
    static std::shared_ptr<Shader> loadShaderFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
protected:
    unsigned int m_programHandle;
    std::unordered_map<std::string, unsigned int> m_uniformLocations;

    unsigned int getUniformLocation(const std::string& name);
};

}; // namespace Codex