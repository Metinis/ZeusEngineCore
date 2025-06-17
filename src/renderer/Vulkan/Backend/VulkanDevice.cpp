#include "VulkanDevice.h"
#include <set>
#include <unordered_set>
#include <ranges>


VulkanDevice::VulkanDevice(vk::Instance instance, vk::SurfaceKHR surface, DispatchLoaderDynamic& loader) :
	m_GPU(findSuitableGpu(instance, surface)),
	m_LogicalDevice(createLogicalDevice(m_GPU, instance, loader))

{
	//std::println("Using GPU: {}", std::string_view{ m_GPU.properties.deviceName });
    
    //init loader with logical device once everything else is init
    loader.init(instance, m_LogicalDevice.get());
}


vk::UniqueDevice VulkanDevice::createLogicalDevice(const GPU& gpu, const vk::Instance instance, DispatchLoaderDynamic& loader)
{
	static constexpr auto queuePriorities_v = std::array{ 1.0f };

    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = gpu.queue_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities_v.data();

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid = gpu.features.fillModeNonSolid;
    deviceFeatures.wideLines = gpu.features.wideLines;
    deviceFeatures.samplerAnisotropy = gpu.features.samplerAnisotropy;
    deviceFeatures.sampleRateShading = gpu.features.sampleRateShading;

    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
    dynamicRenderingFeature.dynamicRendering = VK_TRUE;

    vk::PhysicalDeviceSynchronization2Features syncFeature{};
    syncFeature.synchronization2 = VK_TRUE;
    syncFeature.pNext = &dynamicRenderingFeature;

    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(s_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = s_deviceExtensions.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.pNext = &syncFeature;

    return gpu.device.createDeviceUnique(createInfo);

}

GPU VulkanDevice::findSuitableGpu(vk::Instance const instance, vk::SurfaceKHR const surface)
{
	auto physicalDevices = instance.enumeratePhysicalDevices();

	//check if device supports set extensions
	auto const hasExtSupport = [](GPU const& gpu) {
		auto available = gpu.device.enumerateDeviceExtensionProperties();

		std::unordered_set<std::string_view> names;
		for (const auto& ext : available) names.insert(ext.extensionName);


		return std::all_of(std::begin(s_deviceExtensions), std::end(s_deviceExtensions), [&](const char* ext) {
				return names.contains(std::string_view{ext});
			});
		};

	//check and assign queue families for graphics and transfer
	auto const setQueueFamily = [](GPU& gpu) {
		static constexpr auto queueFlags_v = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer;
		const auto& families = gpu.device.getQueueFamilyProperties();

		for (std::vector<vk::QueueFamilyProperties>::size_type index = 0; index < families.size(); ++index) {
			const auto& family = families[index];
			if ((family.queueFlags & queueFlags_v) == queueFlags_v) {
				gpu.queue_family = static_cast<std::uint32_t>(index);
				return true;
			}
		}
		return false;
	};



	//check if it can present
	auto const canPresent = [surface](GPU const& gpu) {
		return gpu.device.getSurfaceSupportKHR(gpu.queue_family, surface) == vk::True;
	};


	auto fallback = GPU();
	//iterate over all physical devices and find suitable gpu
	for (auto const& device : instance.enumeratePhysicalDevices()) {
		auto gpu = GPU{ .device = device, .properties = device.getProperties() };
        if (!hasExtSupport(gpu)) continue;
        if (!setQueueFamily(gpu)) continue;
        if (!canPresent(gpu)) continue;

		gpu.features = gpu.device.getFeatures();
		if (gpu.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
			return gpu;
		}
		//if discrete gpu found, use it otherwise use fallback
		fallback = gpu;
	}

	if (fallback.device) { return fallback; }
	throw std::runtime_error{ "No suitable Vulkan Physical Devices" };
}



