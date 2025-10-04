#include "cinder.hpp"
#include "hex/framebuffer.hpp"

#include <glad.h>
#include <math.h>

namespace hex {

// Framebuffer
    
Framebuffer::Framebuffer(int width, int height, bool depthOnly) 
    : m_colorTarget(nullptr, width, height, depthOnly ? 1 : 4, true)
{
    glGenFramebuffers(1, &m_framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);

    m_colorTarget.bind(0);
    m_colorTarget.attachToFramebuffer(GL_COLOR_ATTACHMENT0);

    glGenRenderbuffers(1, &m_depthStencilTarget);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilTarget);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilTarget);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cinder::error("Framebuffer is not complete!");
    }
    cinder::log("Created framebuffer.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_framebufferHandle);
    glDeleteRenderbuffers(1, &m_depthStencilTarget);
    cinder::log("Framebuffer destroyed.");
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height) {
    m_colorTarget.resize(width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilTarget);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

// GBuffer

GBuffer::GBuffer(int width, int height) 
    : Framebuffer(width, height) 
    , m_normalTarget(nullptr, width, height, 4, true)
    , m_positionTarget(nullptr, width, height, 4, true)
    , m_aoRoughnessMetallicTarget(nullptr, width, height, 4)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);

    m_normalTarget.bind(1);
    m_normalTarget.attachToFramebuffer(GL_COLOR_ATTACHMENT1);

    m_positionTarget.bind(2);
    m_positionTarget.attachToFramebuffer(GL_COLOR_ATTACHMENT2);

    m_aoRoughnessMetallicTarget.bind(3);
    m_aoRoughnessMetallicTarget.attachToFramebuffer(GL_COLOR_ATTACHMENT3);

    GLenum attachments[6] = {
        GL_COLOR_ATTACHMENT0, 
        GL_COLOR_ATTACHMENT1, 
        GL_COLOR_ATTACHMENT2, 
        GL_COLOR_ATTACHMENT3,
    };
    glDrawBuffers(6, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cinder::error("GBuffer is not complete!");
    }
    cinder::log("Created GBuffer.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GBuffer::~GBuffer() {
    // Base destructor will be called automatically
    cinder::log("GBuffer destroyed.");
}

void GBuffer::resize(int width, int height) {
    m_colorTarget.resize(width, height);
    m_normalTarget.resize(width, height);
    m_positionTarget.resize(width, height);
    m_aoRoughnessMetallicTarget.resize(width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilTarget);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

}