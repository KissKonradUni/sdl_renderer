#include "codex/texture.hpp"
#include "codex/library.hpp"
#include "echo/console.hpp"

#include "lib/glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"

namespace Codex {

TextureData::~TextureData() { 
    stbi_image_free(pixels); 
}

Texture::Texture(unsigned char* pixels, int width, int height, int channels) {
    this->m_data.reset(new TextureData{pixels, width, height, channels});
    this->loadResource();
    this->m_data.reset(); // Seems wasteful, but we must standardize the IResource interface

    this->m_runtimeResource = true;
    this->m_node = nullptr;

    Echo::log("Runtime texture created.");
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
    Echo::log("Texture placeholder created.");
}

Texture::~Texture() {
    glDeleteTextures(1, &m_textureHandle);

    Echo::log("Texture destroyed.");
}

void Texture::loadData(const FileNode* file) {
    if (m_initialized) {
        Echo::warn("Texture already initialized.");
        return;
    }

    if (file == nullptr) {
        Echo::error("No file to load texture from.");
        return;
    }
    
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    auto data = stbi_load((Library::instance().getAssetsRoot() / file->path).c_str(), &width, &height, &channels, 4);

    if (!data) {
        Echo::error(std::string("Failed to load texture from file: ") + file->path.string());
        return;
    }
    Echo::log(std::string("Loaded texture from file: ") + file->path.string());

    this->m_data.reset(new TextureData{data, width, height, channels});
    this->m_node = file;
    this->m_runtimeResource = false;
}

void Texture::loadResource() {    
    if (m_initialized) {
        Echo::warn("Texture already initialized.");
        return;
    }

    if (m_data == nullptr) {
        Echo::error("No texture data to load.");
        return;
    }

    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    this->m_width    = m_data->width;
    this->m_height   = m_data->height;
    this->m_channels = m_data->channels;

    bool empty = m_data->pixels == nullptr;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_data->width, m_data->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, empty ? NULL : m_data->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, empty ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (!empty)
        glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    this->m_initialized = true;
}

void Texture::bind(int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
}

void Texture::resize(int width, int height) {
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::attachToFramebuffer(int attachment) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_textureHandle, 0);
}

} // namespace Codex