#include "GLContext.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include <glad/glad.h>
#include <stdexcept>

using namespace ZEN;

GLContext::GLContext(GLFWwindow* window) : m_WindowHandle(window){
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    glEnable(GL_DEPTH_TEST);
}
void GLContext::drawMesh(const MeshDrawableComp& meshDrawable) {
    //retrieve GLDrawable by meshID from resource manager
    m_ResourceManager.bindMeshDrawable(meshDrawable.meshID);
    glDrawElementsInstanced(GL_TRIANGLES, meshDrawable.indexCount, GL_UNSIGNED_INT, nullptr, 1);
    glBindVertexArray(0);
}

void GLContext::clear(bool shouldClearColor, bool shouldClearDepth) {
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


void GLContext::swapBuffers() {
    glfwSwapBuffers(m_WindowHandle);
}
