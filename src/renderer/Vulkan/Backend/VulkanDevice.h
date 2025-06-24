#pragma once
#include "vulkan/vulkan.hpp"
#include <optional>
#include "../../../Utils.h"

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};
struct GPU {
	vk::PhysicalDevice device{};
	vk::PhysicalDeviceProperties properties{};
	vk::PhysicalDeviceFeatures features{};
	std::uint32_t queueFamily = UINT32_MAX;;
};
class VulkanDevice {
public:
	VulkanDevice(vk::Instance instance, vk::SurfaceKHR surface);
	[[nodiscard]] vk::Device GetLogicalDevice() const { return *m_LogicalDevice; };
	[[nodiscard]] const GPU& GetGPU() const { return m_GPU; };
	void SubmitToQueue(vk::SubmitInfo2 submitInfo, vk::Fence drawn);
	const vk::Queue GetQueue() const { return m_Queue; }
private:
	GPU m_GPU;
	vk::UniqueDevice m_LogicalDevice;
	vk::Queue m_Queue;
#ifdef __APPLE__
	static constexpr std::array<const char*, 3> s_deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		"VK_KHR_portability_subset",
		"VK_EXT_shader_object"
	};
#else
	static constexpr std::array<const char*, 3> s_deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
		"VK_EXT_shader_object"
	};
#endif

	vk::UniqueDevice CreateLogicalDevice(const GPU& gpu, const vk::Instance instance);
	static GPU FindSuitableGpu(vk::Instance const instance, vk::SurfaceKHR const surface);
};