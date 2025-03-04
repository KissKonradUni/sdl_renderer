#include "codex/texture.hpp"
#include "echo/console.hpp"

#include "lib/glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"

namespace Codex {

TextureData::~TextureData() { 
    stbi_image_free(pixels); 
    Echo::log("Texture data freed.");
}

Texture::Texture(const unsigned char* pixels, int width, int height, int channels) {
    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    m_width    = width;
    m_height   = height;
    m_channels = channels;

    bool empty = pixels == nullptr;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, empty ? NULL : pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, empty ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (!empty)
        glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
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

Texture::~Texture() {
    glDeleteTextures(1, &m_textureHandle);

    Echo::log("Texture destroyed.");
}

std::shared_ptr<TextureData> Texture::loadTextureDataFromFile(const std::string& filename) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    auto data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

    if (!data) {
        Echo::error(std::string("Failed to load texture from file: ") + filename);
        return nullptr;
    }
    Echo::log(std::string("Loaded texture from file: ") + filename);

    return std::make_shared<TextureData>(data, width, height, 4);
}

std::shared_ptr<Texture> Texture::loadTextureFromFile(const std::string& filename) {
    auto data = loadTextureDataFromFile(filename);
    if (!data) {
        return nullptr;
    }
    auto result = std::make_shared<Texture>(data->pixels, data->width, data->height, data->channels);
    return result;
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