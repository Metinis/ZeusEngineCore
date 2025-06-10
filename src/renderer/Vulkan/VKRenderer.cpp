#include "VKRenderer.h"
#include <vulkan/vulkan.hpp>

void VKRenderer::Init() {
	std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation"};
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
	vkBackend = std::make_unique<VulkanBackend>(layers);
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