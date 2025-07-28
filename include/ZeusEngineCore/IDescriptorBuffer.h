#pragma once
#include <bit>
#include <span>
#include <memory>

namespace ZEN {
    enum class eDescriptorBufferType{
        UBO,
        SSBO
    };
    class IRendererBackend;
    class IRendererAPI;
    class IDescriptorBuffer {
    public:
        virtual void Write(std::span<std::byte const> bytes) = 0;

        virtual void Bind() = 0;

        static std::unique_ptr<IDescriptorBuffer> Create(IRendererBackend* apiBackend,
                                                         IRendererAPI* apiRenderer,
                                                         eDescriptorBufferType type);

        virtual ~IDescriptorBuffer() = default;
    };
}