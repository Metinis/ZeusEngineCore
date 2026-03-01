#include "ImGUILayerVulkan.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/color_space.hpp>
#include "ZeusEngineCore/core/Application.h"

using namespace ZEN;

ImGUILayerVulkan::ImGUILayerVulkan(GLFWwindow *window) {
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(window, true);

	ImGui_ImplVulkan_InitInfo initInfo = ZEN::Application::get().getVKRenderer()->initImgui();
	//ImGui_ImplVulkan_Init(&initInfo);
    //callback = [this](void* cmd) {
        //this->endFrame(cmd);
    //};
}

ImGUILayerVulkan::~ImGUILayerVulkan() {
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
void ImGUILayerVulkan::beginFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
void ImGUILayerVulkan::render() {
	ImGui::Render();
}

void ImGUILayerVulkan::endFrame(void* commandBuffer) {
	//ImDrawData* data = ImGui::GetDrawData();
	//if (data == nullptr) { return; }
	//VkCommandBuffer cmd = reinterpret_cast<VkCommandBuffer>(commandBuffer);
	//ImGui_ImplVulkan_RenderDrawData(data, vkCommandBuffer);
	//VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	//VkRenderingInfo renderInfo = VKInit::renderingInfo(_swapchainExtent, &colorAttachment, nullptr);

	//vkCmdBeginRendering(cmd, &renderInfo);

	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	//vkCmdEndRendering(cmd);
}

