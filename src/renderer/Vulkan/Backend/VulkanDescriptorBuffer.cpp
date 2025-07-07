#include "VulkanDescriptorBuffer.h"

VulkanDescriptorBuffer::VulkanDescriptorBuffer(const BufferCreateInfo& bufferCreateInfo) :
	m_BufferCreateInfo(bufferCreateInfo)
{
	for (auto& buffer : m_Buffers) { WriteTo(buffer, {}); }
}

void VulkanDescriptorBuffer::WriteAt(std::size_t frameIndex, std::span<std::byte const> bytes)
{
	WriteTo(m_Buffers.at(frameIndex), bytes);
}

vk::DescriptorBufferInfo VulkanDescriptorBuffer::GetDescriptorInfoAt(std::size_t frameIndex)
{
	auto const& buffer = m_Buffers.at(frameIndex);
	vk::DescriptorBufferInfo DescriptorBufferInfo{};
	if(buffer.has_value())
		DescriptorBufferInfo.setBuffer(buffer.value().Get()).setRange(buffer.value().Size());
	return DescriptorBufferInfo;
}

void VulkanDescriptorBuffer::WriteTo(std::optional<VulkanBuffer>& outBuffer, std::span<std::byte const> bytes) const
{
	static constexpr auto blankByte_v = std::array{ std::byte{} };
	// fallback to an empty byte if bytes is empty.
	if (bytes.empty()) { bytes = blankByte_v; }

	if (!outBuffer.has_value() || outBuffer->Size() < bytes.size()) {
		//recreate buffer.
		outBuffer = VulkanBuffer(m_BufferCreateInfo, BufferMemoryType::Host, bytes.size());
	}
	std::memcpy(outBuffer->Mapped(), bytes.data(), bytes.size());
}
