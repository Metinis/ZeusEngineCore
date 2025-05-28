#include "GLRenderer.h"
#include <string>
#include <glad/glad.h>


void GLRenderer::Init() {

}

GLRenderer::~GLRenderer() = default;


void GLRenderer::BeginFrame() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRenderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    //todo probably sort by material to reduce state changes
    m_RenderQueue.emplace_back(transform, material, mesh);
}


void GLRenderer::EndFrame() {
//glfw swapping handled in Window
    //draw submitted commands
    for(const auto& cmd : m_RenderQueue) {
        cmd.mesh->Draw(*cmd.material);
    }
    m_RenderQueue.clear();
}

void GLRenderer::DrawMesh(const IMesh& mesh, Material& material) {
    //immediately draw (costly for performance but option available)
    mesh.Draw(material);
}
