#include "VulkanDevice.h"

VulkanDevice::VulkanDevice(vk::Instance instance)
{
	m_PhysicalDevice = nullptr;

	auto physicalDevices = instance.enumeratePhysicalDevices();
	if (physicalDevices.empty()) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	m_PhysicalDevice = physicalDevices[0];
}
