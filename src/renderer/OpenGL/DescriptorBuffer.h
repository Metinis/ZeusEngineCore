#pragma once
#include "ZeusEngineCore/IDescriptorBuffer.h"

namespace ZEN {
    enum class eDescriptorBufferType;
}
namespace ZEN::OGLAPI {
    struct BufferCreateInfo;
    class APIRenderer;
    class DescriptorBuffer : public IDescriptorBuffer{
    public:
        explicit DescriptorBuffer(const BufferCreateInfo& bufferCreateInfo);
        ~DescriptorBuffer() override = default;
        void Write(std::span<std::byte const> bytes) override;
        void Bind() override;
    };
}
