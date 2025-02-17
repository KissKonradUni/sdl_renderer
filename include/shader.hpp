#pragma once

#include "floatmath.hpp"

#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
    ~Shader();

    void bind();
    
    void setUniform(const std::string& name, const matrix4x4f& value);

    static std::unique_ptr<Shader> load(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
protected:
    unsigned int programHandle;
    std::unordered_map<std::string, unsigned int> uniformLocations;

    unsigned int getUniformLocation(const std::string& name);
};