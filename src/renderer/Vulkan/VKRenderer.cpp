#include "VKRenderer.h"
#include <vulkan/vulkan.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

void VKRenderer::Init(RendererInitInfo& initInfo) {
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
    std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_shader_object" };

    WindowHandle windowHandlePtr;
    if (initInfo.windowHandle.has_value()) {
        windowHandlePtr = initInfo.windowHandle.value();
    }

    m_VKBackend = std::make_unique<VulkanBackend>(layers, windowHandlePtr);
}

VKRenderer::~VKRenderer() = default;


bool VKRenderer::BeginFrame() {
    if(!m_VKBackend->AcquireRenderTarget()){
        return false;
    }
    m_CommandBuffer = m_VKBackend->BeginFrame();
    m_VKBackend->TransitionForRender(m_CommandBuffer);

    return true;

}

void VKRenderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    m_RenderQueue.emplace_back(transform, material, mesh);

}


void VKRenderer::EndFrame(const std::function<void(vk::CommandBuffer)>& uiExtraDrawCallback) {
    std::function<void(vk::CommandBuffer)> drawCallback = [=](vk::CommandBuffer commandBuffer) {
        // Do mesh-specific drawing here
        for(const auto& cmd : m_RenderQueue) {
            cmd.material->Bind(commandBuffer, m_VKBackend->GetFramebufferSize());
            cmd.mesh->Draw(*cmd.material, commandBuffer);
            //commandBuffer.draw(3, 1, 0, 0);
       //     cmd.material->GetShader()->SetUniformMat4("u_Model", cmd.transform);
        //    cmd.mesh->Draw(*cmd.material);
        }
        m_RenderQueue.clear();
    };
    std::function<void(vk::CommandBuffer)> uiDrawCallback = [=](vk::CommandBuffer commandBuffer) {
        // Do ui drawing here
        if (uiExtraDrawCallback) {
            uiExtraDrawCallback(commandBuffer);
        }
    };

    m_VKBackend->Render(m_CommandBuffer, drawCallback, uiDrawCallback);
    m_VKBackend->TransitionForPresent(m_CommandBuffer);
    m_VKBackend->SubmitAndPresent();
}

void VKRenderer::DrawMesh(const IMesh& mesh, Material& material) {
    m_VKBackend->Render(m_CommandBuffer);
}

RendererContextVariant VKRenderer::GetContext() const
{
    VulkanContextInfo contextInfo = m_VKBackend->GetContext();
    return contextInfo;
}

ShaderInfoVariant VKRenderer::GetShaderInfo() const {
    return m_VKBackend->GetShaderInfo();
}
