#pragma once

#include "codex/texture.hpp"

namespace Hex {

class Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    void bind();
    void unbind();

    void resize(int width, int height);

    Codex::Texture& getColorTarget() { return m_colorTarget; }
private:
    unsigned int m_framebufferHandle;

    Codex::Texture m_colorTarget;
    unsigned int m_depthStencilTarget;
};

}; // namespace Hex