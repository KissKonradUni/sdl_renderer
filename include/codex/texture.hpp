#pragma once

#include <string>
#include <memory>

#include "floatmath.hpp"
#include "lib/stb/stb_image.h"

namespace Codex {

struct TextureData {
    unsigned char* pixels;
    int width, height, channels;

    ~TextureData();
};

class Texture {
public:
    Texture(const unsigned char* pixels, int width, int height, int channels);
    Texture(const TextureData* data);
    Texture(const vector4f& color);
    ~Texture();

    /**
     * @brief Loads a texture from a file
     * 
     * @param filename - The path to the file
     * @return TextureData - The texture data
     */
    static std::shared_ptr<TextureData> loadTextureDataFromFile(const std::string& filename);
    static std::shared_ptr<Texture> loadTextureFromFile(const std::string& filename);
    
    void bind(int slot);
    void resize(int width, int height);
    void attachToFramebuffer(int attachment);

    /**
     * @attention This is used for internal purposes, please refrain from using it
     * @return unsigned int - The OpenGL texture handle
     */
    const unsigned int getHandle() const { return m_textureHandle; }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getChannels() const { return m_channels; }
protected:
    unsigned int m_textureHandle;
    int m_width, m_height, m_channels;
};

}; // namespace Codex