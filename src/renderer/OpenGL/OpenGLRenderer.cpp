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

void OpenGLRenderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    m_RenderQueue.emplace_back(transform, material, mesh);
}


void OpenGLRenderer::EndFrame() {
//glfw swapping handled in Window
    //draw submitted commands
    for(const auto& cmd : m_RenderQueue) {
        cmd.mesh->Draw(*cmd.material);
    }
    m_RenderQueue.clear();
}

void OpenGLRenderer::DrawMesh(const IMesh& mesh, Material& material) {
    //immediately draw (costly for performance but option available)
    mesh.Draw(material);
}
