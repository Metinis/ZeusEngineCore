#include "DescriptorBuffer.h"

using namespace ZEN::VKAPI;

DescriptorBuffer::DescriptorBuffer(const BufferCreateInfo& bufferCreateInfo) :
	m_BufferCreateInfo(bufferCreateInfo)
{
	for (auto& buffer : m_Buffers) { WriteTo(buffer, {}); }
}

void DescriptorBuffer::WriteAt(std::size_t frameIndex, std::span<std::byte const> bytes)
{
	WriteTo(m_Buffers.at(frameIndex), bytes);
}

vk::DescriptorBufferInfo DescriptorBuffer::GetDescriptorInfoAt(std::size_t frameIndex) const{
	auto const& buffer = m_Buffers.at(frameIndex);
	vk::DescriptorBufferInfo DescriptorBufferInfo{};
	if(buffer.has_value())
		DescriptorBufferInfo.setBuffer(buffer.value().Get()).setRange(buffer.value().Size());
	return DescriptorBufferInfo;
}

void DescriptorBuffer::WriteTo(std::optional<Buffer>& outBuffer, std::span<std::byte const> bytes) const
{
	static constexpr auto blankByte_v = std::array{ std::byte{} };
	// fallback to an empty byte if bytes is empty.
	if (bytes.empty()) { bytes = blankByte_v; }

	if (!outBuffer.has_value() || outBuffer->Size() < bytes.size()) {
		//recreate buffer.
		outBuffer = Buffer(m_BufferCreateInfo, BufferMemoryType::Host, bytes.size());
	}
	std::memcpy(outBuffer->Mapped(), bytes.data(), bytes.size());
}
