#pragma once
#include "ZeusEngineCore/IDescriptorBuffer.h"
#include <cstdint>


namespace ZEN {
    enum class eDescriptorBufferType;
    enum class GLenum;
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
    private:
        std::uint32_t m_Binding{};
        std::uint32_t m_BufferHandle{};
        std::uint32_t m_Size{};
    };
}
