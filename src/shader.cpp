#include "shader.hpp"
#include "console.hpp"

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
        console->warn("Vertex shader compilation failed...");
        console->warn(infoLog);
    }
    
    auto fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSource = fragmentShaderSource.c_str();
    glShaderSource(fragmentShaderHandle, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShaderHandle);

    glGetShaderiv(fragmentShaderHandle, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShaderHandle, 512, NULL, infoLog);
        console->warn("Fragment shader compilation failed...");
        console->warn(infoLog);
    }

    programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShaderHandle);
    glAttachShader(programHandle, fragmentShaderHandle);
    glLinkProgram(programHandle);

    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(programHandle, 512, NULL, infoLog);
        console->warn("Shader program linking failed...");
        console->warn(infoLog);
    }

    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);

    console->log("Shader program created.");
}

Shader::~Shader() {
    glDeleteProgram(programHandle);

    console->log("Shader program destroyed");
}

void Shader::bind() {
    glUseProgram(programHandle);
}

void Shader::setUniform(const std::string& name, const matrix4x4f& value) {
    const GLint location = getUniformLocation(name);
    if (location == -1) {
        std::string message = "Couldn't find uniform: " + name;
        console->warn(message);
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

unsigned int Shader::getUniformLocation(const std::string& name) {
    auto it = uniformLocations.find(name);
    if (it != uniformLocations.end()) {
        return it->second;
    }

    const unsigned int location = glGetUniformLocation(programHandle, name.c_str());
    uniformLocations[name] = location;
    return location;
}