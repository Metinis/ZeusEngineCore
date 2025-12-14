#include "GLContext.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include <glad/glad.h>
#include "ZeusEngineCore/AssetLibrary.h"

using namespace ZEN;

GLContext::GLContext(GLFWwindow* window)
: m_WindowHandle(window){
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
void GLContext::drawMesh(IResourceManager& resourceManager, const MeshDrawable& drawable) {
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
