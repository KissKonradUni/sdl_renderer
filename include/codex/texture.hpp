#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Codex {

class Texture {
public:
    Texture(std::vector<unsigned char>& data, int width, int height, int channels);
    ~Texture();

    static std::unique_ptr<Texture> loadTextureFromFile(const std::string& filename);
    
    void bind(int slot);
protected:
    unsigned int m_textureHandle;

    int m_width, m_height, m_channels;
};

}; // namespace Codex