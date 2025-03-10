#include "codex/shader.hpp"
#include "echo/console.hpp"

#include "lib/json/json.hpp"
#include "lib/glad/glad.h"

#include <filesystem>
#include <fstream>

namespace Codex {

UniformBuffer::UniformBuffer(size_t size, int binding) {
    glGenBuffers(1, &m_uniformBufferObjectHandle);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferObjectHandle);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_uniformBufferObjectHandle);

    Echo::log("Uniform buffer created.");

    m_size = size;
}

void UniformBuffer::bind() {
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferObjectHandle);
}

void UniformBuffer::updateData(const UniformBufferData* data) {
    bind();
    UniformBufferData* dataPointer = (UniformBufferData*)glMapBufferRange(
        GL_UNIFORM_BUFFER,
        0,
        m_size,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
    );
    if (dataPointer) {
        SDL_memcpy(dataPointer, data, m_size);
    }
    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

ShaderResource::ShaderResource(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        Echo::error(std::string("Shader file not found: ") + path);
        return;
    }

    using nlohmann::json;

    std::ifstream file(path);
    json data = json::parse(file);

    m_name                   = data["name"].template get<std::string>();
    m_vertexShaderFilename   = data["vert"].template get<std::string>();
    m_fragmentShaderFilename = data["frag"].template get<std::string>();

    Echo::log(std::string("Shader resource created: ") + m_name);
}

ShaderResource::~ShaderResource() {
    Echo::log(std::string("Shader resource destroyed: ") + m_name);
}

Shader::Shader(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
    int success;
    char infoLog[512];
    
    auto vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexSource = vertexShaderSource.c_str();
    glShaderSource(vertexShaderHandle, 1, &vertexSource, NULL);
    glCompileShader(vertexShaderHandle);

    glGetShaderiv(vertexShaderHandle, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShaderHandle, 512, NULL, infoLog);
        Echo::warn("Vertex shader compilation failed...");
        Echo::warn(infoLog);
    }
    
    auto fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSource = fragmentShaderSource.c_str();
    glShaderSource(fragmentShaderHandle, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShaderHandle);

    glGetShaderiv(fragmentShaderHandle, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShaderHandle, 512, NULL, infoLog);
        Echo::warn("Fragment shader compilation failed...");
        Echo::warn(infoLog);
    }

    m_programHandle = glCreateProgram();
    glAttachShader(m_programHandle, vertexShaderHandle);
    glAttachShader(m_programHandle, fragmentShaderHandle);
    glLinkProgram(m_programHandle);

    glGetProgramiv(m_programHandle, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(m_programHandle, 512, NULL, infoLog);
        Echo::warn("Shader program linking failed...");
        Echo::warn(infoLog);
    }

    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);

    Echo::log("Shader program created.");
}

Shader::~Shader() {
    glDeleteProgram(m_programHandle);

    Echo::log("Shader program destroyed.");
}

void Shader::bind() {
    glUseProgram(m_programHandle);
}

void Shader::setUniform(const std::string& name, const int& value) {
    const GLint location = getUniformLocation(name);
    if (location == -1) {
        Echo::warn(std::string("Couldn't find uniform: ") + name);
        return;
    }
    glUniform1i(location, value);
}

void Shader::setUniform(const std::string& name, const vector4f& value) {
    const GLint location = getUniformLocation(name);
    if (location == -1) {
        Echo::warn(std::string("Couldn't find uniform: ") + name);
        return;
    }
    glUniform4fv(location, 1, value.as_array.data());
}

void Shader::setUniform(const std::string& name, const matrix4x4f& value) {
    const GLint location = getUniformLocation(name);
    if (location == -1) {
        Echo::warn(std::string("Couldn't find uniform: ") + name);
        return;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, value.as_array.data());
}

std::shared_ptr<ShaderData> Shader::loadShaderDataFromFile(const std::string &vertexShaderFilename, const std::string &fragmentShaderFilename) {
    if (!std::filesystem::exists(vertexShaderFilename)) {
        Echo::error(std::string("Vertex shader file not found: ") + vertexShaderFilename);
        return nullptr;
    }

    if (!std::filesystem::exists(fragmentShaderFilename)) {
        Echo::error(std::string("Fragment shader file not found: ") + fragmentShaderFilename);
        return nullptr;
    }
    
    std::ifstream vertexShaderFile(vertexShaderFilename);
    std::string vertexShaderSource((std::istreambuf_iterator<char>(vertexShaderFile)), std::istreambuf_iterator<char>());

    std::ifstream fragmentShaderFile(fragmentShaderFilename);
    std::string fragmentShaderSource((std::istreambuf_iterator<char>(fragmentShaderFile)), std::istreambuf_iterator<char>());

    return std::make_shared<ShaderData>(ShaderData{vertexShaderSource, fragmentShaderSource});
}

std::shared_ptr<Shader> Shader::loadShaderFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename) {
    auto sources = loadShaderDataFromFile(vertexShaderFilename, fragmentShaderFilename);    
    return std::make_shared<Shader>(sources->vertexShaderSource, sources->fragmentShaderSource);
}

unsigned int Shader::getUniformLocation(const std::string& name) {
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }

    const unsigned int location = glGetUniformLocation(m_programHandle, name.c_str());
    m_uniformLocations[name] = location;
    return location;
}

} // namespace Codex