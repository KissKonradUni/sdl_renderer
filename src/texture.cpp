#include "texture.hpp"
#include "console.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.hpp"

#include "glad/glad.h"

Texture::Texture(std::vector<unsigned char>& data, int width, int height, int channels) {
    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    glDeleteTextures(1, &m_textureHandle);
}

std::unique_ptr<Texture> Texture::loadTextureFromFile(const std::string& filename) {
    int width, height, channels;
    auto data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

    if (!data) {
        console->error(std::string("Failed to load texture from file: ") + filename);
        return nullptr;
    }
    console->log(std::string("Loaded texture from file: ") + filename);

    std::vector<unsigned char> textureData(data, data + width * height * 4);
    auto result = std::make_unique<Texture>(textureData, width, height, 4);
    stbi_image_free(data);

    return result;
}

void Texture::bind(int slot) {
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glActiveTexture(GL_TEXTURE0 + slot);
}