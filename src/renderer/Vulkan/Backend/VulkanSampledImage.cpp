#include "VulkanSampledImage.h"
#include "VulkanBuffer.h"

VulkanSampledImage::VulkanSampledImage(const ImageCreateInfo& createInfo, 
	VulkanCommandBlock commandBlock, Bitmap const& bitmap)
{
	const unsigned int mipLevels = 1u;
	glm::uvec2 uSize{ bitmap.size };
	vk::Extent2D extent{ uSize.x, uSize.y };
	auto const usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	VulkanImage image(createInfo, usage, mipLevels, vk::Format::eR8G8B8A8Srgb, extent);

	//create staging buffer
	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.allocator = createInfo.allocator;
	bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	bufferCreateInfo.queueFamily = createInfo.queueFamily;
	bufferCreateInfo.destroyCallback = createInfo.destroyCallback;

	const VulkanBuffer stagingBuffer(bufferCreateInfo, BufferMemoryType::Host, bitmap.bytes.size_bytes());

	//check if creation failed for image or buffer
	if (!image.Get() || !stagingBuffer.Get()) { return; }

	//copy bytes into staging buffer
	std::memcpy(stagingBuffer.Mapped(), bitmap.bytes.data(), bitmap.bytes.size_bytes());

	//transition image for transfer
	vk::DependencyInfo dependencyInfo{};
	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subresourceRange.setLayerCount(1);
	subresourceRange.setLevelCount(mipLevels);

	vk::ImageMemoryBarrier2 barrier{};
	barrier.setImage(image.Get());
	barrier.setSrcQueueFamilyIndex(createInfo.queueFamily);
	barrier.setDstQueueFamilyIndex(createInfo.queueFamily);
	barrier.setOldLayout(vk::ImageLayout::eUndefined);
	barrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
	barrier.setSubresourceRange(subresourceRange);
	barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe);
	barrier.setSrcAccessMask(vk::AccessFlagBits2::eNone);
	barrier.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer);
	barrier.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite);
	dependencyInfo.setImageMemoryBarriers(barrier);
	commandBlock.GetCommandBuffer().pipelineBarrier2(dependencyInfo);

	//record image copy into buffer
	vk::BufferImageCopy2 bufferImageCopy{};
	vk::ImageSubresourceLayers subresourceLayers{};
	subresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subresourceLayers.setLayerCount(1);
	subresourceLayers.setMipLevel(0);

	bufferImageCopy.setImageSubresource(subresourceLayers);
	bufferImageCopy.setImageExtent(vk::Extent3D{ extent.width, extent.height, 1 });

	vk::CopyBufferToImageInfo2 copyInfo{};
	copyInfo.setDstImage(image.Get());
	copyInfo.setDstImageLayout(vk::ImageLayout::eTransferDstOptimal);
	copyInfo.setSrcBuffer(stagingBuffer.Get());
	copyInfo.setRegions(bufferImageCopy);;
	commandBlock.GetCommandBuffer().copyBufferToImage2(copyInfo);

	//transition for sampling, submit and wait#
	barrier.setOldLayout(barrier.newLayout);
	barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	barrier.setSrcStageMask(barrier.dstStageMask);
	barrier.setSrcAccessMask(barrier.dstAccessMask);
	barrier.setDstStageMask(vk::PipelineStageFlagBits2::eAllGraphics);
	barrier.setDstAccessMask(vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite);

	dependencyInfo.setImageMemoryBarriers(barrier);

	commandBlock.GetCommandBuffer().pipelineBarrier2(dependencyInfo);

	commandBlock.submitAndWait();

	m_SampledImage = std::move(image);
}
