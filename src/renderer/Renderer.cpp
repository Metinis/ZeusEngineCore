#include "ZeusEngineCore/Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include "ZeusEngineCore/Material.h"
#include "ZeusEngineCore/IMesh.h"
#include "ZeusEngineCore/Utils.h"
#include "ZeusEngineCore/IRendererBackend.h"
#include "ZeusEngineCore/IRendererAPI.h"
#include "ZeusEngineCore/IDescriptorBuffer.h"

using namespace ZEN;

Renderer::Renderer(RendererInitInfo &initInfo) {
    if(!initInfo.windowHandle.nativeWindowHandle)
        throw std::runtime_error("Null window handle!");

    m_Backend = IRendererBackend::Create(initInfo.api, initInfo.windowHandle);
    m_APIRenderer = IRendererAPI::Create(initInfo.api, m_Backend.get());
    m_ViewUBO = IDescriptorBuffer::Create(m_Backend.get(), m_APIRenderer.get(), eDescriptorBufferType::UBO);
}

Renderer::~Renderer() = default;


bool Renderer::BeginFrame() {
    if(!m_APIRenderer->BeginFrame()){
        return false;
    }
    UpdateView();
    return true;
}

void Renderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    m_RenderQueue.emplace_back(transform, material, mesh);

}


void Renderer::EndFrame(const std::function<void(void*)>& uiExtraDrawCallback) {
    m_ViewUBO->Bind();
    for(const auto& cmd : m_RenderQueue){
        cmd.material->Bind();
        cmd.mesh->Draw();
    }
    m_RenderQueue.clear();
    if(uiExtraDrawCallback){
        m_APIRenderer->DrawWithCallback(uiExtraDrawCallback); //injects command buffer
    }
    m_APIRenderer->SubmitAndPresent();
}

void Renderer::UpdateView()
{
    auto const halfSize = 0.5f * glm::vec2{1280, 720};
    glm::mat4 projectionMat = glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y);
    glm::mat4 viewMat = m_ViewTransform.viewMatrix();
    glm::mat4 vpMat = projectionMat * viewMat;
    auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vpMat)>>(vpMat);
    m_ViewUBO->Write(bytes);
}

IRendererAPI* Renderer::GetAPIRenderer() const {
    return m_APIRenderer.get();
}

IRendererBackend* Renderer::GetAPIBackend() const {
    return m_Backend.get();
}


