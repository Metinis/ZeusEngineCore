#include "DescriptorBuffer.h"
#include "APIRenderer.h"
#include <utility>

using namespace ZEN::VKAPI;

DescriptorBuffer::DescriptorBuffer(BufferCreateInfo  bufferCreateInfo) :
	m_BufferCreateInfo(std::move(bufferCreateInfo))
{
    m_APIRenderer = bufferCreateInfo.apiRenderer;
	for (auto& buffer : m_Buffers) { WriteTo(buffer, {}); }
}

void DescriptorBuffer::Write(std::span<std::byte const> bytes)
{
	WriteTo(m_Buffers.at(m_APIRenderer->GetFrameIndex()), bytes);
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

void DescriptorBuffer::Bind() {
    if (m_BufferCreateInfo.usage & vk::BufferUsageFlagBits::eStorageBuffer) {
        m_APIRenderer->SetSSBO(*this);
    }
    else if (m_BufferCreateInfo.usage & vk::BufferUsageFlagBits::eUniformBuffer) {
        m_APIRenderer->SetUBO(*this);
    }
    else {
        throw std::runtime_error("DescriptorBuffer::Bind: Unsupported buffer usage flag!");
    }


}
