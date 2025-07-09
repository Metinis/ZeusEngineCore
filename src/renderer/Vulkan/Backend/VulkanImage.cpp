#include "VulkanImage.h"

VulkanImage& VulkanImage::operator=(VulkanImage&& other) noexcept {
	if (this != &other) {
		Destroy();
		m_Handle = other.m_Handle;
		m_DestroyCallback = std::move(other.m_DestroyCallback);

		other.m_Handle = {};
		other.m_DestroyCallback = nullptr;
	}
	return *this;
}

void VulkanImage::Destroy() {
	if (m_Handle.allocator && m_Handle.image && m_Handle.allocation) {
		if (m_DestroyCallback) {
			m_DestroyCallback(m_Handle);
		}
		else {
			std::printf("Empty destroy image callback!");
		}

	}
	m_Handle = {};
	m_DestroyCallback = nullptr;
}

VulkanImage::VulkanImage(ImageCreateInfo const& createInfo, vk::ImageUsageFlags usage,
	std::uint32_t levels, vk::Format format, vk::Extent2D extent)
{
	if (extent.width == 0 || extent.height == 0) {
		//todo use println
		std::printf("Images cannot have 0 width or height");
		return;
	}
	vk::ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.setExtent({ extent.width, extent.height, 1 });
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setFormat(format);
	imageCreateInfo.setUsage(usage);
	imageCreateInfo.setArrayLayers(1);
	imageCreateInfo.setMipLevels(levels);
	imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1); //1 sample per pixel, no multisampling
	imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
	imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
	imageCreateInfo.setQueueFamilyIndices(createInfo.queueFamily);

	const VkImageCreateInfo vkImageCreateInfo = static_cast<VkImageCreateInfo>(imageCreateInfo);

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	VkImage image{};
	VmaAllocation allocation{};
	const VkResult result = vmaCreateImage(createInfo.allocator, &vkImageCreateInfo,
		&allocationCreateInfo, &image, &allocation, {});
	if (result != VK_SUCCESS) {
		std::printf("Failed to create VMA Image");
		return;
	}

	m_Handle.allocator = createInfo.allocator;
	m_Handle.allocation = allocation;
	m_Handle.image = image;
	m_Handle.extent = extent;
	m_Handle.format = format;
	m_Handle.levels = levels;

    if(createInfo.destroyCallback)
	    m_DestroyCallback = *createInfo.destroyCallback; //deferred so images in use are not used

}

