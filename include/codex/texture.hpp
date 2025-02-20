#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Codex {

class Texture {
public:
    Texture(std::vector<unsigned char>& data, int width, int height, int channels);
    Texture(unsigned char* data, int width, int height, int channels);
    ~Texture();

    static std::shared_ptr<Texture> loadTextureFromFile(const std::string& filename);
    
    void bind(int slot);
    void resize(int width, int height);
    void attachToFramebuffer(int attachment);

    /**
     * @attention This is used for internal purposes, please refrain from using it
     * @return unsigned int - The OpenGL texture handle
     */
    unsigned int getHandle() const { return m_textureHandle; }
protected:
    unsigned int m_textureHandle;
    int m_width, m_height, m_channels;
};

}; // namespace Codex