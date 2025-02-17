#include "shader.hpp"

#include "glad/glad.h"

#include <fstream>

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
        SDL_Log("Vertex shader compilation failed: %s", infoLog);
    }
    
    auto fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSource = fragmentShaderSource.c_str();
    glShaderSource(fragmentShaderHandle, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShaderHandle);

    glGetShaderiv(fragmentShaderHandle, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShaderHandle, 512, NULL, infoLog);
        SDL_Log("Fragment shader compilation failed: %s", infoLog);
    }

    programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShaderHandle);
    glAttachShader(programHandle, fragmentShaderHandle);
    glLinkProgram(programHandle);

    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(programHandle, 512, NULL, infoLog);
        SDL_Log("Shader program linking failed: %s", infoLog);
    }

    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);

    SDL_Log("Shader program created");
}

Shader::~Shader() {
    glDeleteProgram(programHandle);

    SDL_Log("Shader program destroyed");
}

void Shader::bind() {
    glUseProgram(programHandle);
}

void Shader::setUniform(const std::string& name, const matrix4x4f& value) {
    const GLint location = glGetUniformLocation(programHandle, name.c_str());
    if (location == -1) {
        SDL_Log("Uniform %s not found", name.c_str());  
        return;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, value.as_array.data());
}

std::unique_ptr<Shader> Shader::load(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename) {
    std::ifstream vertexShaderFile(vertexShaderFilename);
    std::string vertexShaderSource((std::istreambuf_iterator<char>(vertexShaderFile)), std::istreambuf_iterator<char>());

    std::ifstream fragmentShaderFile(fragmentShaderFilename);
    std::string fragmentShaderSource((std::istreambuf_iterator<char>(fragmentShaderFile)), std::istreambuf_iterator<char>());

    return std::make_unique<Shader>(vertexShaderSource, fragmentShaderSource);
}