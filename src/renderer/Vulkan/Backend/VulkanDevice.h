#pragma once
#include "vulkan/vulkan.hpp"
class VulkanDevice {
public:
	VulkanDevice(vk::Instance instance);
private:
	vk::PhysicalDevice m_PhysicalDevice;
};