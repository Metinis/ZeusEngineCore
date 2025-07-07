#pragma once
#include "VulkanImage.h"
#include "VulkanCommandBlock.h"

class VulkanSampledImage {
public:
	VulkanSampledImage(const ImageCreateInfo& createInfo, VulkanCommandBlock commandBlock,
		Bitmap const& bitmap);
	const VulkanImage& Get() { return *m_SampledImage; }
private:
	std::optional<VulkanImage> m_SampledImage;
};