#pragma once
#include "VulkanBuffer.h"
#include "VulkanCommandBlock.h"

using ByteSpans = std::span<std::span<std::byte const> const>;

class VulkanDeviceBuffer {
public:
	VulkanDeviceBuffer(BufferCreateInfo const& createInfo,
		VulkanCommandBlock commandBlock, ByteSpans const& byteSpans);
	const VulkanBuffer& Get() { return *m_DeviceBuffer; }
private:
	std::optional<VulkanBuffer> m_DeviceBuffer;
};