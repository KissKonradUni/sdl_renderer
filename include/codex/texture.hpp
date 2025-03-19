#pragma once

#include <string>
#include <memory>

#include "floatmath.hpp"
#include "lib/stb/stb_image.h"

namespace Codex {

/**
 * @brief The raw data of a texture
 */
struct TextureData {
    unsigned char* pixels;
    int width, height, channels;

    ~TextureData();
};

/**
 * @brief A texture resource
 * Handles loading and uploading textures to the GPU
 */
class Texture {
friend class Assets;
friend class Material;
public:
    Texture(const unsigned char* pixels, int width, int height, int channels);
    Texture(const TextureData* data);
    Texture(const vector4f& color);
    ~Texture();

    /**
     * @brief Loads a texture's raw data from a file
     * 
     * @param filename - The path to the file
     * @return TextureData - The texture data
     */
    static std::shared_ptr<TextureData> loadTextureDataFromFile(const std::string& filename);
    /**
     * @brief Loads a texture from a file,
     * and uploads it to the GPU
     * @param filename - The path to the file
     * @return std::shared_ptr<Texture> - The texture
     */
    static std::shared_ptr<Texture> loadTextureFromFile(const std::string& filename);
    
    /**
     * @brief Binds the texture to a texture slot
     * @param slot - The slot to bind the texture to
     */
    void bind(int slot);
    /**
     * @brief Resizes the texture
     * 
     * @param width - The new width
     * @param height - The new height
     */
    void resize(int width, int height);
    /**
     * @brief Attaches the texture to a framebuffer
     * @remark This is used for internal purposes, but may be useful for custom framebuffers.
     * @param attachment - The attachment point
     */
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