#include "VKRenderer.h"
#include <vulkan/vulkan.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

void VKRenderer::Init(RendererInitInfo& initInfo) {
    VULKAN_HPP_DEFAULT_DISPATCHER.init();
    std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };

    WindowHandle windowHandlePtr;
    if (initInfo.windowHandle.has_value()) {
        windowHandlePtr = initInfo.windowHandle.value();
    }

    m_VKBackend = std::make_unique<VulkanBackend>(layers, windowHandlePtr);
}

VKRenderer::~VKRenderer() = default;


bool VKRenderer::BeginFrame() {
    //if (!m_VKBackend->AcquireRenderTarget()) {
        //std::println("failed to aquire render target");
    //}
    if(!m_VKBackend->AcquireRenderTarget()){
        return false;
    }
    m_CommandBuffer = m_VKBackend->BeginFrame();
    m_VKBackend->TransitionForRender(m_CommandBuffer);

    return true;

}

void VKRenderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    m_VKBackend->Render(m_CommandBuffer);
}


void VKRenderer::EndFrame() {
    m_VKBackend->TransitionForPresent(m_CommandBuffer);
    m_VKBackend->SubmitAndPresent();
}

void VKRenderer::DrawMesh(const IMesh& mesh, Material& material) {
    m_VKBackend->Render(m_CommandBuffer);
}