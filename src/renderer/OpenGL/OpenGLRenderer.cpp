#include "OpenGLRenderer.h"
#include <string>
#include <glad/glad.h>


void OpenGLRenderer::Init() {

}

void OpenGLRenderer::Cleanup() {
    //glDeleteVertexArrays(1, &m_VAO);
    //glDeleteBuffers(1, &m_VBO);
}

void OpenGLRenderer::BeginFrame() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::Submit() {

}


void OpenGLRenderer::EndFrame() {
//glfw swapping handled in Window
    //draw submitted commands

}

void OpenGLRenderer::DrawMesh(const IMesh& mesh, Material& material) {
    //immediately draw (costly for performance but option available)
   // m_Shader->Bind();
    //m_Shader->SetUniformVec4("u_Color", color);
    //glBindVertexArray(m_VAO);
    //glDrawArrays(GL_TRIANGLES, 0, 3);
    mesh.Draw(material);
}
