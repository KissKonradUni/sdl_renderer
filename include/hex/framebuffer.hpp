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
protected:
    unsigned int m_framebufferHandle;

    codex::Texture m_colorTarget;
    unsigned int m_depthStencilTarget;
};

class GBuffer : public Framebuffer {
public:
    GBuffer(int width, int height);
    ~GBuffer();

    void resize(int width, int height);

    inline const codex::Texture& getNormalTarget() const { return m_normalTarget; }
    inline const codex::Texture& getPositionTarget() const { return m_positionTarget; }
    inline const codex::Texture& getAORoughnessMetallicTarget() const { return m_aoRoughnessMetallicTarget; }
private:
    codex::Texture m_normalTarget;
    codex::Texture m_positionTarget;
    codex::Texture m_aoRoughnessMetallicTarget;
};

}; // namespace hex