#pragma once

#include "floatmath.hpp"

#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace Codex {

struct alignas(16) UniformBufferData {};

class UniformBuffer {
public:
    UniformBuffer(size_t size, int binding);

    void bind();
    void updateData(const UniformBufferData* data);
protected:
    unsigned int m_uniformBufferObjectHandle;
    size_t m_size;
};

class Shader {
public:
    Shader(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
    ~Shader();

    void bind();
    
    void setUniform(const std::string& name, const int& value);
    void setUniform(const std::string& name, const vector4f& value);
    void setUniform(const std::string& name, const matrix4x4f& value);

    static std::unique_ptr<Shader> load(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
protected:
    unsigned int m_programHandle;
    std::unordered_map<std::string, unsigned int> m_uniformLocations;

    unsigned int getUniformLocation(const std::string& name);
};

}; // namespace Codex