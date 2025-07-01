#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <cstdint>

struct BufferCreateInfo {
    VmaAllocator allocator;
    vk::BufferUsageFlags usage;
    std::uint32_t queue_family;
};

enum class BufferMemoryType : std::int8_t { Host, Device };

class VulkanBuffer {
public:
    VulkanBuffer(BufferCreateInfo const& create_info, BufferMemoryType const memory_type, vk::DeviceSize const size);
    // No copy
    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    // Move
    VulkanBuffer(VulkanBuffer&& other) noexcept {
        *this = std::move(other);
    }
    VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;
    ~VulkanBuffer() {
        Destroy();
    }
    std::span<std::byte> mappedSpan() const {
        return {static_cast<std::byte*>(m_Mapped), static_cast<size_t>(m_Size)};
    }
    vk::Buffer Get() const { return m_Buffer; }
    vk::DeviceSize Size() const { return m_Size; }
    void* Mapped() const { return m_Mapped; }
    bool Valid() const { return m_Buffer && m_Allocation; }
private:
    VmaAllocator m_Allocator{};
    VmaAllocation m_Allocation{};
    vk::Buffer m_Buffer{};
    vk::DeviceSize m_Size{};
    void* m_Mapped{};

    void Destroy();
};

