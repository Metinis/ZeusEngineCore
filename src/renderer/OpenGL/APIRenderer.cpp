#include "APIRenderer.h"
#include "APIBackend.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"


using namespace ZEN::OGLAPI;

APIRenderer::APIRenderer(OGLAPI::APIBackend* apiBackend) : m_Backend(apiBackend) {
    
    //Generate MSAA FBO
    glGenFramebuffers(1, &m_MSAAFBO);

    int maxSamples = GetMaxMSAA();
    //Generate MSAA Texture
    SetMSAA(maxSamples);

    
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
    if(uiExtraDrawCallback)
        uiExtraDrawCallback(nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
}

void APIRenderer::SubmitAndPresent() {
    //Swap and render any data in default framebuffer
    SwapBuffers();

    //Copy MSAA FBO Data into default FBO, rendered later
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSAAFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y,
        0, 0, m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );
    
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
int APIRenderer::GetMaxMSAA() {
    int maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    return maxSamples;
}
void APIRenderer::SetMSAA(int msaa)
{
    m_MSAA = msaa;
    glDeleteTextures(1, &m_MSAATex);
    glDeleteRenderbuffers(1, &m_MSAADepthBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);

    glGenTextures(1, &m_MSAATex);
    glGenRenderbuffers(1, &m_MSAADepthBuffer);
    if (msaa) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAATex);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, GL_RGB,
            m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MSAATex, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, m_MSAADepthBuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, GL_DEPTH24_STENCIL8,
            m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAADepthBuffer);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, m_MSAATex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
            m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MSAATex, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, m_MSAADepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
            m_Backend->GetFramebufferSize().x, m_Backend->GetFramebufferSize().y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAADepthBuffer);
    }
}
void APIRenderer::SwapBuffers() {
    glfwSwapBuffers(m_Backend->GetWindowHandle().nativeWindowHandle);
}

