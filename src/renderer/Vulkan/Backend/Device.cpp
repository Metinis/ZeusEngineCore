#include "Device.h"
#include <set>
#include <unordered_set>
#include <ranges>

using namespace ZEN::VKAPI;

constexpr std::array deviceExtensions_v{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
#ifdef __APPLE__
        "VK_KHR_portability_subset",
        VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
#endif
};

Device::Device(vk::Instance instance, vk::SurfaceKHR surface) :
	m_GPU(FindSuitableGpu(instance, surface)),
	m_LogicalDevice(CreateLogicalDevice(m_GPU, instance)),
	m_Queue(m_LogicalDevice->getQueue(m_GPU.queueFamily, 0))

{
	
}

void Device::SubmitToQueue(vk::SubmitInfo2 submitInfo, vk::Fence drawn)
{
	m_Queue.submit2(submitInfo, drawn);
}


vk::UniqueDevice Device::CreateLogicalDevice(const GPU& gpu, const vk::Instance instance)
{
	static constexpr auto queuePriorities_v = std::array{ 1.0f };

    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = gpu.queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities_v.data();

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid = gpu.features.fillModeNonSolid;
    deviceFeatures.wideLines = gpu.features.wideLines;
    deviceFeatures.samplerAnisotropy = gpu.features.samplerAnisotropy;
    deviceFeatures.sampleRateShading = gpu.features.sampleRateShading;

    vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features{};
    dynamicState3Features.extendedDynamicState3PolygonMode = vk::True;
	dynamicState3Features.extendedDynamicState3RasterizationSamples = vk::True;

	vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{vk::True};
    dynamicRenderingFeature.pNext = &dynamicState3Features;

    vk::PhysicalDeviceSynchronization2Features syncFeature{};
    syncFeature.synchronization2 = VK_TRUE;
    syncFeature.pNext = &dynamicRenderingFeature;

    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions_v.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions_v.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.pNext = &syncFeature;

    vk::UniqueDevice device = gpu.device.createDeviceUnique(createInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
    return device;

}

GPU Device::FindSuitableGpu(vk::Instance const instance, vk::SurfaceKHR const surface)
{
	auto physicalDevices = instance.enumeratePhysicalDevices();

	//check if device supports set extensions
	auto const hasExtSupport = [](GPU const& gpu) {
		auto available = gpu.device.enumerateDeviceExtensionProperties();

		std::unordered_set<std::string_view> names;
		for (const auto& ext : available) names.insert(ext.extensionName);


		return std::all_of(std::begin(deviceExtensions_v), std::end(deviceExtensions_v), [&](const char* ext) {
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
				gpu.queueFamily = static_cast<std::uint32_t>(index);
				return true;
			}
		}
		return false;
	};



	//check if it can present
	auto const canPresent = [surface](GPU const& gpu) {
		return gpu.device.getSurfaceSupportKHR(gpu.queueFamily, surface) == vk::True;
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



