#include "GLRenderer.h"
#include <string>
#include <glad/glad.h>


void GLRenderer::Init(RendererInitInfo& initInfo) {

}

GLRenderer::~GLRenderer() = default;


bool GLRenderer::BeginFrame() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return true;
}

void GLRenderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    //todo probably sort by material to reduce state changes
    m_RenderQueue.emplace_back(transform, material, mesh);
}


void GLRenderer::EndFrame(const std::function<void(vk::CommandBuffer)>& uiExtraDrawCallback) {
//glfw swapping handled in Window
    // Reuse a dummy VAO for the entire frame
    static GLuint dummyVAO = 0;
    if (dummyVAO == 0) {
        glGenVertexArrays(1, &dummyVAO);
    }
    glBindVertexArray(dummyVAO);

    // Draw submitted commands
    for (const auto& cmd : m_RenderQueue) {
        cmd.material->Bind(); // assumes this binds the correct shader
        glDrawArrays(GL_TRIANGLES, 0, 3); // rely on gl_VertexIndex inside shader
    }

    glBindVertexArray(0);
    m_RenderQueue.clear();
}

void GLRenderer::DrawMesh(const IMesh& mesh, Material& material) {
    //immediately draw (costly for performance but option available)
    mesh.Draw(material);
}

ShaderInfoVariant GLRenderer::GetShaderInfo() const {
    return ShaderInfoVariant {};
}
