#include "ImGUILayerVulkan.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "ZeusEngineCore/core/Application.h"

using namespace ZEN;

ImGUILayerVulkan::ImGUILayerVulkan() {
}

void ImGUILayerVulkan::init(EngineContext *ctx) {
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(ctx->window->getNativeWindow(), true);
	ImGui_ImplVulkan_InitInfo initInfo = ctx->vkRenderer->initImgui();
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

extern "C" ZEN_API ImGuiContext* getEngineImGuiContext() {
	return ImGui::GetCurrentContext();
}

