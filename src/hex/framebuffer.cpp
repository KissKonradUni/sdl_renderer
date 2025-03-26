#include "hex/framebuffer.hpp"
#include "echo/console.hpp"

#include <glad.h>

namespace hex {
    
Framebuffer::Framebuffer(int width, int height) 
    : m_colorTarget(nullptr, width, height, 4)
{
    glGenFramebuffers(1, &m_framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferHandle);

    m_colorTarget.bind(0);
    m_colorTarget.attachToFramebuffer(GL_COLOR_ATTACHMENT0);

    glGenRenderbuffers(1, &m_depthStencilTarget);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilTarget);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilTarget);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        echo::error("Framebuffer is not complete!");
    }
    echo::log("Created framebuffer.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_framebufferHandle);
    glDeleteRenderbuffers(1, &m_depthStencilTarget);
    echo::log("Framebuffer destroyed.");
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

}