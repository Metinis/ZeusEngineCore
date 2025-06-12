#pragma once
#include "vulkan/vulkan.hpp"
#include <optional>
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};
class VulkanDevice {
public:
	VulkanDevice(vk::Instance instance, vk::SurfaceKHR surface);
private:
	vk::Instance m_Instance;
	vk::SurfaceKHR m_Surface;
	vk::PhysicalDevice m_PhysicalDevice;
	vk::UniqueDevice m_LogicalDevice;
	vk::Queue m_GraphicsQueue;
	vk::Queue m_PresentQueue;
	static constexpr std::array<const char*, 1> s_deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	void CreatePhysicalDevice();
	void CreateLogicalDevice();
	static const bool IsDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
	static QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
	static bool CheckDeviceExtensionSupport(vk::PhysicalDevice, std::span<const char* const> deviceExtensions);
};