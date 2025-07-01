#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include "ZeusEngineCore/ScopedWaiter.h"
#include "VulkanBuffer.h"

class VulkanMemAlloc{
public:
    VulkanMemAlloc(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice);
    ~VulkanMemAlloc();
    VmaAllocator Get() const {return m_Allocator;}
    VulkanBuffer CreateBuffer(BufferCreateInfo const& createInfo, BufferMemoryType const memoryType, vk::DeviceSize const size);
private:
    VmaAllocator m_Allocator;
    vk::Device m_Device;
};