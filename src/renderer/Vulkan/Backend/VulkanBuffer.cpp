#include "VulkanBuffer.h"

VulkanBuffer &VulkanBuffer::operator=(VulkanBuffer &&other) noexcept {
    if (this != &other) {
        Destroy();
        m_Allocator = other.m_Allocator;
        m_Buffer = other.m_Buffer;
        m_Allocation = other.m_Allocation;
        m_Size = other.m_Size;
        m_Mapped = other.m_Mapped;

        other.m_Allocator = nullptr;
        other.m_Buffer = nullptr;
        other.m_Allocation = nullptr;
        other.m_Size = 0;
        other.m_Mapped = nullptr;
    }
    return *this;
}

void VulkanBuffer::Destroy() {
    if (m_Allocator && m_Buffer && m_Allocation) {
        vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
    }
    m_Buffer = nullptr;
    m_Allocation = nullptr;
    m_Mapped = nullptr;
    m_Size = 0;
}

VulkanBuffer::VulkanBuffer(const BufferCreateInfo &createInfo, const BufferMemoryType memoryType,
                           const vk::DeviceSize size) {
    if(size == 0){std::printf("Buffer cannot be 0");}
    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    auto usage = createInfo.usage;
    if(memoryType == BufferMemoryType::Device){
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        usage |= vk::BufferUsageFlagBits::eTransferDst;
    }
    else{
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.setQueueFamilyIndices(createInfo.queue_family);
    bufferCreateInfo.setSize(size);
    bufferCreateInfo.setUsage(usage);

    auto vmaBufferCreateInfo = static_cast<VkBufferCreateInfo>(bufferCreateInfo);
    VmaAllocation allocation{};
    VkBuffer buffer{};
    VmaAllocationInfo allocationInfo{};
    auto const result = vmaCreateBuffer(createInfo.allocator, &vmaBufferCreateInfo,&allocationCreateInfo, &buffer, &allocation,
                                        &allocationInfo);
    if (result != VK_SUCCESS) {
        std::printf("Failed to create VMA Buffer");
        //return {};
    }
    m_Allocator = createInfo.allocator;
    m_Allocation = allocation;
    m_Buffer = buffer;
    m_Size = size;
    m_Mapped = allocationInfo.pMappedData;
}
