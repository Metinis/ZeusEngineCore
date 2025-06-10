#include "VulkanDevice.h"
#include <set>

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
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
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
	createInfo.enabledExtensionCount = 0;

	m_LogicalDevice = m_PhysicalDevice.createDeviceUnique(createInfo);

	m_GraphicsQueue = m_LogicalDevice->getQueue(indices.graphicsFamily.value(), 0);
	m_PresentQueue = m_LogicalDevice->getQueue(indices.presentFamily.value(), 0);
}

const bool VulkanDevice::IsDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
	//vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
	//vk::PhysicalDeviceFeatures features = physicalDevice.getFeatures();
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
	return indices.isComplete();
}
QueueFamilyIndices VulkanDevice::FindQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
	QueueFamilyIndices indices;
	std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
			indices.graphicsFamily = i;
		}
		vk::Bool32 presentSupport = device.getSurfaceSupportKHR(i, surface);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
		i++;
	}
	return indices;
}
