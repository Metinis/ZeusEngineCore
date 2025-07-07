#pragma once
#include "VulkanBuffer.h"
#include "VulkanSync.h"

class VulkanDescriptorBuffer {
public:
	using Buffers = Buffered<std::optional<VulkanBuffer>>;
	VulkanDescriptorBuffer(const BufferCreateInfo& bufferCreateInfo);
	void WriteAt(std::size_t frameIndex, std::span<std::byte const> bytes);
	vk::DescriptorBufferInfo GetDescriptorInfoAt(std::size_t frameIndex);
private:
	BufferCreateInfo m_BufferCreateInfo{};
	Buffers m_Buffers;
	vk::BufferUsageFlags m_Usage{};

	void WriteTo(std::optional<VulkanBuffer>& outBuffer, std::span<std::byte const> bytes) const;
};