#include "Renderer.h"
#include <vulkan/vulkan.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace ZEN::VKAPI;
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

//todo make renderer api agnostic
void Renderer::Init(RendererInitInfo& initInfo) {

    //todo move to backend interface when creating vulkan backend instance
    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    if(!initInfo.windowHandle.nativeWindowHandle)
        throw std::runtime_error("Null window handle!");

    m_VKBackend = std::make_unique<VKAPI::Backend>(initInfo.windowHandle);
    m_ViewUBO.emplace(m_VKBackend->CreateUBO());
    ZEN::TextureInfo textureInfo{};
    textureInfo.textInfoVariant = m_VKBackend->GetTextureInfo();
    m_Texture.Init(textureInfo);
}

Renderer::~Renderer() = default;


bool Renderer::BeginFrame() {
    UpdateView();
    if(!m_VKBackend->AcquireRenderTarget()){
        return false;
    }
    m_CommandBuffer = m_VKBackend->BeginFrame();
    m_VKBackend->TransitionForRender(m_CommandBuffer);

    return true;

}

void Renderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    m_RenderQueue.emplace_back(transform, material, mesh);

}


void Renderer::EndFrame(const std::function<void(vk::CommandBuffer)>& uiExtraDrawCallback) {
    std::function<void(vk::CommandBuffer)> drawCallback = [=, this](vk::CommandBuffer commandBuffer) {
        // Do mesh-specific drawing here
        for(const auto& cmd : m_RenderQueue) {
            cmd.material->Bind(commandBuffer, m_VKBackend->GetExtent());
            m_VKBackend->BindDescriptorSets(commandBuffer, m_ViewUBO.value(), m_Texture.GetDescriptorInfo());
            cmd.mesh->Draw(*cmd.material, commandBuffer);
        }
        m_RenderQueue.clear();
    };
    std::function<void(vk::CommandBuffer)> uiDrawCallback = [=](vk::CommandBuffer commandBuffer) {
        // Do ui drawing here or anything without depth
        if (uiExtraDrawCallback) {
            uiExtraDrawCallback(commandBuffer);
        }
    };

    m_VKBackend->Render(m_CommandBuffer, drawCallback, uiDrawCallback);
    m_VKBackend->TransitionForPresent(m_CommandBuffer);
    m_VKBackend->SubmitAndPresent();
}

void Renderer::DrawMesh(const IMesh& mesh, Material& material) {
    m_VKBackend->Render(m_CommandBuffer);
}

ZEN::RendererContextVariant Renderer::GetContext() const
{
    ContextInfo contextInfo = m_VKBackend->GetContext();
    return contextInfo;
}

ZEN::ShaderInfoVariant Renderer::GetShaderInfo() const {
    return m_VKBackend->GetShaderInfo();
}

void Renderer::UpdateView()
{
    auto const halfSize = 0.5f * glm::vec2{ m_VKBackend->GetFramebufferSize()};
    auto const matProjection = glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y);
    auto const bytes = std::bit_cast<std::array<std::byte, sizeof(matProjection)>>(matProjection);
    m_ViewUBO->WriteAt(m_VKBackend->GetFrameIndex(), bytes);
}
