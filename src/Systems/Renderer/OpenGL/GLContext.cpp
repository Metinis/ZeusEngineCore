#include "GLContext.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include <glad/glad.h>

#include "ZeusEngineCore/core/Application.h"
#include "ZeusEngineCore/asset/AssetLibrary.h"

using namespace ZEN;

GLContext::GLContext() : m_WindowHandle(Application::get().getWindow()->getNativeWindow()){
    glfwMakeContextCurrent(m_WindowHandle);
    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    const int requiredMajor = 4;
    const int requiredMinor = 5;

    if (major < requiredMajor || (major == requiredMajor && minor < requiredMinor)) {
        char buf[128];
        snprintf(buf, sizeof(buf),
            "OpenGL version %d.%d unsupported. Required at least %d.%d",
            major, minor, requiredMajor, requiredMinor);
        throw std::runtime_error(buf);
    }
    glfwSwapInterval(1); // Enable VSync
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
void GLContext::drawMesh(IResourceManager& resourceManager, const GPUMesh& drawable) {
    //retrieve GLDrawable by meshID from resource manager
    resourceManager.bindMeshDrawable(drawable.drawableID);
    glDrawElementsInstanced(GL_TRIANGLES, drawable.indexCount, GL_UNSIGNED_INT,
        nullptr, drawable.instanceCount);
    glBindVertexArray(0);
}

void GLContext::clear(bool shouldClearColor, bool shouldClearDepth) {
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    GLenum clearBit = 0;
    if(shouldClearColor)
        clearBit |= GL_COLOR_BUFFER_BIT;
    if (shouldClearDepth)
        clearBit |= GL_DEPTH_BUFFER_BIT;
    if (clearBit) {
        glClear(clearBit);
    }
}

void GLContext::clearInt() {
    GLuint clearValue = 0;
    glClearBufferuiv(GL_COLOR, 0, &clearValue);
}

void GLContext::depthMask(bool val) {
    glDepthMask(val);

}

void GLContext::enableCullFace() {
    glEnable(GL_CULL_FACE);
}

void GLContext::disableCullFace() {
    glDisable(GL_CULL_FACE);
}

void GLContext::setDepthMode(eDepthModes depthMode) {
    switch (depthMode) {
        case LEQUAL:
            glDepthFunc(GL_LEQUAL);
            return;
        case LESS:
            glDepthFunc(GL_LESS);
            return;
        default:
            return;
    }
}
void GLContext::setViewport(uint32_t width, uint32_t height) {
    glViewport(0, 0, width, height);
}

void GLContext::setViewport(uint32_t xCorner, uint32_t yCorner, uint32_t width, uint32_t height) {
    glViewport(xCorner, yCorner, width, height);
}


void GLContext::swapBuffers() {
    glfwSwapBuffers(m_WindowHandle);
}

uint32_t GLContext::readPixels(FBO fbo, float x, float y) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fboID);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    uint32_t id;
    glReadPixels(
    x,
    y,
    1,
    1,
    GL_RED_INTEGER,
    GL_UNSIGNED_INT,
    &id
    );
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    return id;
}
