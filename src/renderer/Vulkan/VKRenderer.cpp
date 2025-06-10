#include "VKRenderer.h"
void VKRenderer::Init(RendererInitInfo& initInfo) {
    std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };

    WindowHandle* windowHandlePtr = nullptr;
    if (initInfo.windowHandle.has_value()) {
        windowHandlePtr = &(*initInfo.windowHandle);
    }

    vkBackend = std::make_unique<VulkanBackend>(layers, windowHandlePtr);
}

VKRenderer::~VKRenderer() = default;


void VKRenderer::BeginFrame() {

}

void VKRenderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {

}


void VKRenderer::EndFrame() {

}

void VKRenderer::DrawMesh(const IMesh& mesh, Material& material) {

}