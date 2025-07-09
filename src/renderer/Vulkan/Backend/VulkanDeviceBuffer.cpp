#include "VulkanDeviceBuffer.h"


VulkanDeviceBuffer::VulkanDeviceBuffer(BufferCreateInfo const& createInfo,
	VulkanCommandBlock commandBlock, ByteSpans const& byteSpans)
{
	std::size_t totalSize = 0;
	for (auto const& bytes : byteSpans) {
		totalSize += bytes.size();
	}
	auto stagingCreateInfo = createInfo;
	stagingCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;

	VulkanBuffer stagingBuffer(stagingCreateInfo, BufferMemoryType::Host, totalSize);

	VulkanBuffer deviceBuffer(createInfo, BufferMemoryType::Device, totalSize);

	if (!stagingBuffer.Get() || !deviceBuffer.Get()) { return; }

	//copy bytes into staging buffer
	auto dst = stagingBuffer.mappedSpan();
	for (auto const& bytes : byteSpans) {
		std::memcpy(dst.data(), bytes.data(), bytes.size());
		dst = dst.subspan(bytes.size());
	}


	vk::BufferCopy2 bufferCopy{};
	bufferCopy.setSize(totalSize);
	vk::CopyBufferInfo2 copyBufferInfo{};
	copyBufferInfo.setSrcBuffer(stagingBuffer.Get());
	copyBufferInfo.setDstBuffer(deviceBuffer.Get());
	copyBufferInfo.setRegions(bufferCopy);
    commandBlock.GetCommandBuffer().copyBuffer2(copyBufferInfo);

	commandBlock.submitAndWait();

	m_DeviceBuffer = std::move(deviceBuffer);
}
