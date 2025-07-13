#include "Renderer.h"
#include <string>
#include <glad/glad.h>

using namespace ZEN::OGLAPI;

void Renderer::Init(ZEN::RendererInitInfo& initInfo) {

}

Renderer::~Renderer() = default;


bool Renderer::BeginFrame() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return true;
}

void Renderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    //todo probably sort by material to reduce state changes
    m_RenderQueue.emplace_back(transform, material, mesh);
}


void Renderer::EndFrame(const std::function<void(vk::CommandBuffer)>& uiExtraDrawCallback) {
//glfw swapping handled in Window
    for(const auto& cmd : m_RenderQueue) {
        cmd.mesh->Draw(*cmd.material);
    }
    m_RenderQueue.clear();

    if(uiExtraDrawCallback){
        uiExtraDrawCallback(nullptr);
    }
}

void Renderer::DrawMesh(const IMesh& mesh, Material& material) {
    //immediately draw (costly for performance but option available)
    mesh.Draw(material);
}

ZEN::ShaderInfoVariant Renderer::GetShaderInfo() const {
    return ShaderInfoVariant {};
}
