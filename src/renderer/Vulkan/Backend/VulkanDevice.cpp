#include "VulkanDevice.h"
#include <set>
#include <unordered_set>
#include <ranges>

VulkanDevice::VulkanDevice(vk::Instance instance, vk::SurfaceKHR surface) : m_Instance(instance), m_Surface(surface)
{
	CreatePhysicalDevice();
	CreateLogicalDevice();
}

void VulkanDevice::CreatePhysicalDevice()
{
	m_PhysicalDevice = nullptr;

	auto physicalDevices = m_Instance.enumeratePhysicalDevices();

	for (const auto& device : physicalDevices) {
		if (IsDeviceSuitable(device, m_Surface)) {
			m_PhysicalDevice = device;
			break;
		}
	}
	if (m_PhysicalDevice == nullptr) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
}

void VulkanDevice::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice, m_Surface);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	auto uniqueQueueFamilies = std::unordered_set{
	indices.graphicsFamily.value(),
	indices.presentFamily.value()
	};


	float queuePriority = 1.0f;
	for (auto queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures{};
	vk::DeviceCreateInfo createInfo{};

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = s_deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = s_deviceExtensions.data();

	m_LogicalDevice = m_PhysicalDevice.createDeviceUnique(createInfo);

	m_GraphicsQueue = m_LogicalDevice->getQueue(indices.graphicsFamily.value(), 0);
	m_PresentQueue = m_LogicalDevice->getQueue(indices.presentFamily.value(), 0);
}

const bool VulkanDevice::IsDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
	return indices.isComplete() && CheckDeviceExtensionSupport(physicalDevice, std::span{ s_deviceExtensions });
}
QueueFamilyIndices VulkanDevice::FindQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
	QueueFamilyIndices indices;
	auto queueFamilies = device.getQueueFamilyProperties();
	for (size_t i = 0; i < queueFamilies.size(); ++i) {
		const auto& queueFamily = queueFamilies[i];

		if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && !indices.graphicsFamily.has_value()) {
			indices.graphicsFamily = i;
		}

		if (device.getSurfaceSupportKHR(i, surface) && !indices.presentFamily.has_value()) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
	}
	return indices;
}


bool VulkanDevice::CheckDeviceExtensionSupport(vk::PhysicalDevice physicalDevice, std::span<const char* const> deviceExtensions)
{
	auto available = physicalDevice.enumerateDeviceExtensionProperties();

	std::unordered_set<std::string_view> names;
	for (const auto& ext : available) names.insert(ext.extensionName);

	return std::ranges::all_of(deviceExtensions, [&](std::string_view ext) {
		return names.contains(ext);
		});
}

