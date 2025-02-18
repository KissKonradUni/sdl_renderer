#include "codex/texture.hpp"
#include "echo/console.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"

#include "lib/glad/glad.h"

namespace Codex {

Texture::Texture(std::vector<unsigned char>& data, int width, int height, int channels) 
    : Texture(data.data(), width, height, channels) {}

Texture::~Texture() {
    glDeleteTextures(1, &m_textureHandle);
}

std::unique_ptr<Texture> Texture::loadTextureFromFile(const std::string& filename) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    auto data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

    if (!data) {
        Echo::error(std::string("Failed to load texture from file: ") + filename);
        return nullptr;
    }
    Echo::log(std::string("Loaded texture from file: ") + filename);

    std::vector<unsigned char> textureData(data, data + width * height * 4);
    auto result = std::make_unique<Texture>(textureData, width, height, 4);
    stbi_image_free(data);

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

Texture::Texture(unsigned char* data, int width, int height, int channels) {
    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    m_width = width;
    m_height = height;
    m_channels = channels;

    bool empty = data == nullptr;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, empty ? NULL : data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, empty ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (!empty)
        glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Codex