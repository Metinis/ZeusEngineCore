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

    m_WindowHandle = initInfo.windowHandle;

    m_Backend = IRendererBackend::Create(initInfo.api, initInfo.windowHandle);
    m_APIRenderer = IRendererAPI::Create(initInfo.api, m_Backend.get());
    m_ViewUBO = IDescriptorBuffer::Create(m_Backend.get(), m_APIRenderer.get(), eDescriptorBufferType::UBO);
    m_InstanceSSBO = IDescriptorBuffer::Create(m_Backend.get(), m_APIRenderer.get(), eDescriptorBufferType::SSBO);
}

Renderer::~Renderer() = default;


bool Renderer::BeginFrame() {
    if(!m_APIRenderer->BeginFrame()){
        return false;
    }
    m_APIRenderer->SetMSAA(4);
    m_APIRenderer->Clear(true, true);
    UpdateView();
    return true;
}

void Renderer::Submit(const std::vector<Transform>& transforms, const std::shared_ptr<IMesh> &mesh,
                      const std::shared_ptr<Material>& material) {
    m_RenderQueue.emplace_back(transforms, mesh, material);
}


void Renderer::EndFrame(const std::function<void(void*)>& uiExtraDrawCallback) {
    //m_APIRenderer->SetMSAA(4);
    m_APIRenderer->SetDepth(true);
    m_ViewUBO->Bind();
    m_InstanceSSBO->Bind();

    for (const auto& cmd : m_RenderQueue) {
        cmd.material->Bind();
        cmd.mesh->Draw(cmd.transforms.size());
    }
    m_RenderQueue.clear();

    m_APIRenderer->SetMSAA(0);
    m_APIRenderer->SetDepth(false);

    if (uiExtraDrawCallback) {
        m_APIRenderer->DrawWithCallback(uiExtraDrawCallback);
    }

    m_APIRenderer->SubmitAndPresent();
}

void Renderer::UpdateView()
{
    glm::mat4 projectionMat = m_Backend->GetPerspectiveMatrix(65.0f, 0.1f, 100.0f);
    glm::mat4 viewMat = m_ViewTransform.viewMatrix();
    glm::mat4 vpMat = projectionMat * viewMat;
    auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vpMat)>>(vpMat);
    m_ViewUBO->Write(bytes);
}
void Renderer::UpdateInstances(const std::vector<Transform> &instances) {
    std::vector<glm::mat4> instanceData{};
    instanceData.reserve(instances.size());
    for(auto const& instance : instances){
        instanceData.push_back(instance.modelMatrix());
    }
    auto const span = std::span{instanceData};
    void* data = span.data();
    auto const bytes = std::span{static_cast<std::byte const*>(data), span.size_bytes()};
    m_InstanceSSBO->Write(bytes);
}
IRendererAPI* Renderer::GetAPIRenderer() const {
    return m_APIRenderer.get();
}

IRendererBackend* Renderer::GetAPIBackend() const {
    return m_Backend.get();
}




