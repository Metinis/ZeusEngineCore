#include "GLContext.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include <glad/glad.h>
#include <stdexcept>

using namespace ZEN;

GLContext::GLContext(GLFWwindow* window) {
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
}
void GLContext::DrawMesh(const MeshDrawableComp& meshDrawable) {
    glBindVertexArray(meshDrawable.gl.vao);
    glDrawElementsInstanced(GL_TRIANGLES, meshDrawable.indexCount, GL_UNSIGNED_INT, nullptr, 1);
    glBindVertexArray(0);
}
void GLContext::SwapBuffers() {
    glfwSwapBuffers(m_WindowHandle);
}