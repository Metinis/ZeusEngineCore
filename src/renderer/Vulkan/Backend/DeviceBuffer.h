#pragma once
#include "Buffer.h"

namespace ZEN::VKAPI {
    class CommandBlock;

    using ByteSpans = std::span<std::span<std::byte const> const>;

    class DeviceBuffer {
    public:
        DeviceBuffer(BufferCreateInfo const &createInfo,
                     CommandBlock commandBlock, ByteSpans const &byteSpans);

        [[nodiscard]] const Buffer &Get() const { return *m_DeviceBuffer; }

    private:
        std::optional<Buffer> m_DeviceBuffer;
    };
}