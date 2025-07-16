#pragma once
#include <vulkan/vulkan.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include "vma/vk_mem_alloc.h"
#include "ZeusEngineCore/InfoVariants.h"

namespace ZEN::VKAPI {
    class APIRenderer;
    struct BufferHandle {
        VmaAllocator allocator;
        VkBuffer buffer;
        VmaAllocation allocation;
    };
    struct BufferCreateInfo {
        VmaAllocator allocator;
        vk::BufferUsageFlags usage;
        std::uint32_t queueFamily;
        std::shared_ptr<std::function<void(DeferredHandle)>> destroyCallback;
        APIRenderer* apiRenderer;
    };
    enum class BufferMemoryType : std::int8_t {
        Host, Device
    };


    class Buffer {
    public:
        Buffer(BufferCreateInfo createInfo, BufferMemoryType memoryType, vk::DeviceSize size);

        // No copy
        Buffer(const Buffer &) = delete;

        Buffer &operator=(const Buffer &) = delete;

        // Move
        Buffer(Buffer &&other) noexcept {
            *this = std::move(other);
        }

        Buffer &operator=(Buffer &&other) noexcept;

        ~Buffer() {
            Destroy();
        }

        [[nodiscard]] std::span<std::byte> mappedSpan() const {
            return {static_cast<std::byte *>(m_Mapped), static_cast<size_t>(m_Size)};
        }

        [[nodiscard]] vk::Buffer Get() const { return m_Handle.buffer; }

        [[nodiscard]] vk::DeviceSize Size() const { return m_Size; }

        void SetSize(const vk::DeviceSize size) { m_Size = size; }

        [[nodiscard]] void *Mapped() const { return m_Mapped; }

        [[nodiscard]] bool Valid() const { return m_Handle.buffer && m_Handle.allocation; }

    private:
        void Destroy();

        BufferHandle m_Handle{};
        vk::DeviceSize m_Size{};
        void *m_Mapped{};
        std::function<void(DeferredHandle)> m_DestroyCallback;
    };
}

