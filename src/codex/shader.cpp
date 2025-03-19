#include "codex/library.hpp"
#include "codex/shader.hpp"
#include "echo/console.hpp"

#include "lib/glad/glad.h"
#include "lib/json/json.hpp"

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

Shader::Shader() {
    m_programHandle = -1;

    m_data = nullptr;
    m_node = nullptr;
    m_runtimeResource = true;
    m_initialized = false;

    Echo::log("Shader program placeholder created.");
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

void Shader::loadData(const FileNode* file) {
    m_node = file;

    if (m_initialized) {
        Echo::warn("Shader program already initialized.");
        return;
    }

    if (file == nullptr) {
        Echo::error("No file to load shader from.");
        return;
    }
    
    using namespace nlohmann;
    auto& library = Codex::Library::instance();

    std::ifstream metaFile(library.getAssetsRoot() / file->path);
    json meta = json::parse(metaFile);

    std::filesystem::path vertexShaderFilename   = meta["vert"];
    std::filesystem::path fragmentShaderFilename = meta["frag"];
    library.formatPath(&vertexShaderFilename);
    library.formatPath(&fragmentShaderFilename);

    if (!std::filesystem::exists(library.getAssetsRoot() / vertexShaderFilename)) {
        Echo::error("Vertex shader file not found: " + vertexShaderFilename.string());
        return;
    }
    if (!std::filesystem::exists(library.getAssetsRoot() / fragmentShaderFilename)) {
        Echo::error("Fragment shader file not found: " + fragmentShaderFilename.string());
        return;
    }

    std::ifstream vertexShaderFile(library.getAssetsRoot() / vertexShaderFilename);
    std::ifstream fragmentShaderFile(library.getAssetsRoot() / fragmentShaderFilename);

    m_data = std::make_unique<ShaderData>();
    m_data->vertexShaderSource = std::string(
        (std::istreambuf_iterator<char>(vertexShaderFile)),
        std::istreambuf_iterator<char>()
    );
    m_data->fragmentShaderSource = std::string(
        (std::istreambuf_iterator<char>(fragmentShaderFile)),
        std::istreambuf_iterator<char>()
    );

    m_runtimeResource = false;
    Echo::log("Shader data loaded.");
}

void Shader::loadResource() {
    int success;
    char infoLog[512];

    if (m_initialized) {
        Echo::warn("Shader program already initialized.");
        return;
    }

    if (m_data == nullptr) {
        Echo::error("Shader data is null.");
        return;
    }

    auto vertexShaderSource = m_data->vertexShaderSource;
    auto fragmentShaderSource = m_data->fragmentShaderSource;
    
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
    m_initialized = true;
    m_data.reset();
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