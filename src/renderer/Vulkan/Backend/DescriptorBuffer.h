#pragma once
#include "Buffer.h"
#include "Sync.h"

namespace ZEN::VKAPI {
    class DescriptorBuffer {
    public:
        using Buffers = Buffered<std::optional<Buffer>>;

        DescriptorBuffer(const BufferCreateInfo &bufferCreateInfo);

        void WriteAt(std::size_t frameIndex, std::span<std::byte const> bytes);

        vk::DescriptorBufferInfo GetDescriptorInfoAt(std::size_t frameIndex);

    private:
        BufferCreateInfo m_BufferCreateInfo{};
        Buffers m_Buffers;
        vk::BufferUsageFlags m_Usage{};

        void WriteTo(std::optional<Buffer> &outBuffer, std::span<std::byte const> bytes) const;
    };
}