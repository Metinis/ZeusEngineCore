#include "ImGUILayerVulkan.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
//#include <imgui_impl_vulkan.h>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/color_space.hpp>

using namespace ZEN;

ImGUILayerVulkan::ImGUILayerVulkan(GLFWwindow *window) {
    /*IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    static auto const loadVkFunc = +[](char const* name, void* userData) {
        return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(
                *static_cast<vk::Instance*>(userData), name);
    };

    auto instance = backendInfo.instance;
    ImGui_ImplVulkan_LoadFunctions(backendInfo.apiVersion, loadVkFunc, &instance);

    if (!ImGui_ImplGlfw_InitForVulkan(window, true)) {
        throw std::runtime_error{ "Failed to initialize Dear ImGui" };
    }

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = backendInfo.apiVersion;
    initInfo.Instance = backendInfo.instance;
    initInfo.PhysicalDevice = backendInfo.physicalDevice;
    initInfo.Device = backendInfo.device;
    initInfo.QueueFamily = backendInfo.queueFamily;
    initInfo.Queue = backendInfo.queue;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<std::uint32_t>(ZEN::buffering_v);
    initInfo.MSAASamples =
            static_cast<VkSampleCountFlagBits>(backendInfo.samples);
    initInfo.DescriptorPoolSize = 32;

    auto pipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.setColorAttachmentCount(1)
    .setColorAttachmentFormats(backendInfo.colorFormat);

    initInfo.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;
    initInfo.UseDynamicRendering = true;

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        throw std::runtime_error{ "Failed to initialize Dear ImGui" };
    }

    ImGui::StyleColorsDark();
    for (auto& colour : ImGui::GetStyle().Colors) {
        auto const linear = glm::convertSRGBToLinear(
                glm::vec4{ colour.x, colour.y, colour.z, colour.w });
        colour = ImVec4{ linear.x, linear.y, linear.z, linear.w };
    }
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.99f;
    //used to submit to renderer
    callback = [this](void* cmd) {
        this->EndFrame(cmd);
    };*/
}

ImGUILayerVulkan::~ImGUILayerVulkan() {
	//ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
void ImGUILayerVulkan::beginFrame() {
	ImGui_ImplGlfw_NewFrame();
	//ImGui_ImplVulkan_NewFrame();

	ImGui::NewFrame();
}
void ImGUILayerVulkan::render() {
	ImGui::Render();
}

void ImGUILayerVulkan::endFrame(void* commandBuffer) {
	ImDrawData* data = ImGui::GetDrawData();
	if (data == nullptr) { return; }
	//VkCommandBuffer vkCommandBuffer = reinterpret_cast<VkCommandBuffer>(commandBuffer);
	//ImGui_ImplVulkan_RenderDrawData(data, vkCommandBuffer);
}

