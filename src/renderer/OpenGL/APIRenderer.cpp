#include "APIRenderer.h"
#include "APIBackend.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"


using namespace ZEN::OGLAPI;

APIRenderer::APIRenderer(OGLAPI::APIBackend* apiBackend) : m_Backend(apiBackend) {


    //Generate MSAA Texture
    GLint maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

    glGenTextures(1, &m_MSAATex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAATex);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, maxSamples, GL_RGB,
        m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y, GL_TRUE);

    glGenRenderbuffers(1, &m_MSAADepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_MSAADepthBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, maxSamples, GL_DEPTH24_STENCIL8,
        m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y);
    //Generate MSAA FBO
    glGenFramebuffers(1, &m_MSAAFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MSAATex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAADepthBuffer);
}

bool APIRenderer::BeginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
    glViewport(0, 0, m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y);
    //begin cant fail on opengl
    return true;
}

void APIRenderer::DrawWithCallback(const std::function<void(void *)> &uiExtraDrawCallback) {
    //Render to FBO (non msaa)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Clear(true, false);
    if(uiExtraDrawCallback)
        uiExtraDrawCallback(nullptr);

    //Copy into MSAA FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_MSAAFBO);
    glBlitFramebuffer(
        0, 0, m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y,
        0, 0, m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
}

void APIRenderer::SubmitAndPresent() {
    //Copy MSAA FBO Data into default FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSAAFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y,
        0, 0, m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );
    SwapBuffers();
}

void APIRenderer::SetDepth(bool isDepth) {
    if(isDepth)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    glDepthFunc(GL_LESS);
}

void APIRenderer::Clear(bool shouldClearColor, bool shouldClearDepth) {
    //todo set clear color dynamically
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    GLenum clearBit = 0;
    if(shouldClearColor)
        clearBit |= GL_COLOR_BUFFER_BIT;
    if (shouldClearDepth)
        clearBit |= GL_DEPTH_BUFFER_BIT;
    if (clearBit) {
        glClear(clearBit);
    }
    
}
void APIRenderer::SwapBuffers() {
    glfwSwapBuffers(m_Backend->GetWindowHandle().nativeWindowHandle);
}

