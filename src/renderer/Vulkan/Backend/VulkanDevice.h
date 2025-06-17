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
	std::uint32_t queue_family = UINT32_MAX;;
};
class VulkanDevice {
public:
	VulkanDevice(vk::Instance instance, vk::SurfaceKHR surface, DispatchLoaderDynamic& loader);
	[[nodiscard]] vk::Device getLogicalDevice() const { return *m_LogicalDevice; };
	[[nodiscard]] const GPU& getGPU() const { return m_GPU; };
private:
	GPU m_GPU;
	vk::UniqueDevice m_LogicalDevice;
	vk::Queue m_Queue;
#ifdef __APPLE__
	static constexpr std::array<const char*, 2> s_deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		"VK_KHR_portability_subset"
	};
#else
	static constexpr std::array<const char*, 1> s_deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
#endif

	vk::UniqueDevice createLogicalDevice(const GPU& gpu, const vk::Instance instance, DispatchLoaderDynamic& loader);
	static GPU findSuitableGpu(vk::Instance const instance, vk::SurfaceKHR const surface);
};