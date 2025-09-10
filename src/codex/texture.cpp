#include "cinder.hpp"
#include "codex/texture.hpp"
#include "codex/library.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad.h>

namespace codex {

TextureData::~TextureData() { 
    stbi_image_free(pixels); 
}

Texture::Texture(unsigned char* pixels, int width, int height, int channels, bool highPrecision) {
    this->m_highPrecision = highPrecision;
    this->m_data.reset(new TextureData{pixels, width, height, channels});
    this->loadResource();
    this->m_data.reset(); // Seems wasteful, but we must standardize the IResource interface

    this->m_runtimeResource = true;
    this->m_node = nullptr;

    cinder::log("Runtime texture created.");
}

Texture::Texture(const TextureData* data) : Texture(data->pixels, data->width, data->height, data->channels) {}

Texture::Texture(const vector4f& color) : Texture(nullptr, 1, 1, 4) {
    unsigned char data[4] = {
        static_cast<unsigned char>(SDL_clamp(color.x, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(SDL_clamp(color.y, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(SDL_clamp(color.z, 0.0f, 1.0f) * 255.0f),
        static_cast<unsigned char>(SDL_clamp(color.w, 0.0f, 1.0f) * 255.0f)
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

Texture::Texture() {
    cinder::log("Texture placeholder created.");
}

Texture::~Texture() {
    glDeleteTextures(1, &m_textureHandle);

    cinder::log("Texture destroyed.");
}

void Texture::loadData(const FileNode* file) {
    if (m_initialized) {
        cinder::warn("Texture already initialized.");
        return;
    }

    if (file == nullptr) {
        cinder::error("No file to load texture from.");
        return;
    }
    
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    auto data = stbi_load((cinder::app->getLibrary()->getAssetsRoot() / file->path).string().c_str(), &width, &height, &channels, 4);

    if (!data) {
        cinder::error(std::string("Failed to load texture from file: ") + file->path.string());
        return;
    }
    cinder::log(std::string("Loaded texture from file: ") + file->path.string());

    this->m_data.reset(new TextureData{data, width, height, channels});
    this->m_node = file;
    this->m_runtimeResource = false;
}

void Texture::loadResource() {    
    if (m_initialized) {
        cinder::warn("Texture already initialized.");
        return;
    }

    if (m_data == nullptr) {
        cinder::error("No texture data to load.");
        return;
    }

    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    this->m_width    = m_data->width;
    this->m_height   = m_data->height;
    this->m_channels = m_data->channels;

    bool empty = m_data->pixels == nullptr;

    unsigned int internalFormat = m_highPrecision ? GL_RGBA16F : GL_RGBA8;
    unsigned int type = m_highPrecision ? GL_FLOAT : GL_UNSIGNED_BYTE;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_data->width, m_data->height,
                 0, GL_RGBA, type, empty ? NULL : m_data->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, empty ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (!empty)
        glGenerateMipmap(GL_TEXTURE_2D);

    // Check if the texture was created successfully
    int success;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &success);
    if (success == 0) {
        cinder::error("Failed to create texture on GPU.");
        glDeleteTextures(1, &m_textureHandle);
        m_textureHandle = 0;
        return;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    this->m_initialized = true;
    this->m_data.reset(); // We don't need the data anymore
}

void Texture::bind(int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
}

void Texture::resize(int width, int height) {
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    unsigned int internalFormat = m_highPrecision ? GL_RGBA16F : GL_RGBA8;
    unsigned int type = m_highPrecision ? GL_FLOAT : GL_UNSIGNED_BYTE;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, type, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::attachToFramebuffer(int attachment) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_textureHandle, 0);
}

} // namespace codex