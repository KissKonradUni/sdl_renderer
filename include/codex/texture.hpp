#pragma once

#include "codex/resource.hpp"
#include "floatmath.hpp"

#include <stb_image.h>

namespace codex {

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
class Texture : public IResource<TextureData> {
public:
    Texture(unsigned char* pixels, int width, int height, int channels, bool highPrecision = false);
    Texture(const TextureData* data);
    Texture(const vector4f& color);
    Texture();
    virtual ~Texture();

    /**
     * @brief Loads a texture's raw data from a file
     *
     * @param file - The file node to load the texture from
     */
    void loadData(const FileNode* file);
    /**
     * @brief Uploads the texture to the GPU
     */
    void loadResource();
    
    /**
     * @brief Binds the texture to a texture slot
     * @param slot - The slot to bind the texture to
     */
    void bind(int slot) const;
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
    bool m_highPrecision = false;
};

}; // namespace codex