#pragma once
#include "Buffer.h"
#include "Sync.h"
#include "ZeusEngineCore/IDescriptorBuffer.h"

namespace ZEN::VKAPI {
    class DescriptorBuffer : public IDescriptorBuffer {
    public:
        using Buffers = Buffered<std::optional<Buffer>>;

        explicit DescriptorBuffer(BufferCreateInfo  bufferCreateInfo);

        void Write(std::span<std::byte const> bytes) override;

        void Bind() override;

        vk::DescriptorBufferInfo GetDescriptorInfoAt(std::size_t frameIndex) const;

    private:
        BufferCreateInfo m_BufferCreateInfo{};
        Buffers m_Buffers;
        APIRenderer* m_APIRenderer;

        void WriteTo(std::optional<Buffer> &outBuffer, std::span<std::byte const> bytes) const;
    };
}