#pragma once

#include "codex/texture.hpp"

namespace hex {

class Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    void bind();
    void unbind();

    void resize(int width, int height);

    inline const codex::Texture& getColorTarget() const { return m_colorTarget; }
private:
    unsigned int m_framebufferHandle;

    codex::Texture m_colorTarget;
    unsigned int m_depthStencilTarget;
};

}; // namespace hex