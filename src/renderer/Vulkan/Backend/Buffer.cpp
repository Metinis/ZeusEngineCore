#include <functional>
#include "Buffer.h"
#include "Image.h"

using namespace ZEN::VKAPI;

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        Destroy();
        m_Handle = other.m_Handle;
        m_Size = other.m_Size;
        m_Mapped = other.m_Mapped;
        m_DestroyCallback = std::move(other.m_DestroyCallback);

        other.m_Handle = {};
        other.m_Size = 0;
        other.m_Mapped = nullptr;
        other.m_DestroyCallback = nullptr;
    }
    return *this;
}

void Buffer::Destroy() {
    if (m_Handle.allocator && m_Handle.buffer && m_Handle.allocation) {
        if(m_DestroyCallback){
            m_DestroyCallback(m_Handle);

        }
        else{
            std::printf("Empty destroy buffer callback!");
        }

    }
    m_Handle = {};
    m_Size = 0;
    m_Mapped = nullptr;
    m_DestroyCallback = nullptr;
}

Buffer::Buffer(const BufferCreateInfo createInfo, const BufferMemoryType memoryType,
               const vk::DeviceSize size) {
    if (size == 0) {
        std::printf("Buffer cannot be 0");
    }
    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    auto usage = createInfo.usage;
    if (memoryType == BufferMemoryType::Device) {
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        usage |= vk::BufferUsageFlagBits::eTransferDst;
    } else {
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.setQueueFamilyIndices(createInfo.queueFamily);
    bufferCreateInfo.setSize(size);
    bufferCreateInfo.setUsage(usage);

    auto vmaBufferCreateInfo = static_cast<VkBufferCreateInfo>(bufferCreateInfo);
    VmaAllocation allocation{};
    VkBuffer buffer{};
    VmaAllocationInfo allocationInfo{};
    auto const result = vmaCreateBuffer(createInfo.allocator, &vmaBufferCreateInfo, &allocationCreateInfo, &buffer, &allocation,
                                        &allocationInfo);
    if (result != VK_SUCCESS) {
        std::printf("Failed to create VMA Buffer");
        // Handle error or throw exception here if appropriate
    }

    m_Handle.allocator = createInfo.allocator;
    m_Handle.buffer = buffer;
    m_Handle.allocation = allocation;
    if(createInfo.destroyCallback)
        m_DestroyCallback = *createInfo.destroyCallback;
    m_Size = size;
    m_Mapped = allocationInfo.pMappedData;
}
