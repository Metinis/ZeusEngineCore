#include "VulkanMemAlloc.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

VulkanMemAlloc::VulkanMemAlloc(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice)
: m_Device(logicalDevice){
    VmaVulkanFunctions vmaVkFunctions{};
    vmaVkFunctions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
    vmaVkFunctions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = logicalDevice;
    allocatorCreateInfo.pVulkanFunctions = &vmaVkFunctions;
    allocatorCreateInfo.instance = instance;
    auto const result = vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator);
    if(result != VK_SUCCESS){
        throw std::runtime_error{"Failed to create Vulkan Memory Allocator"};
    }
}

VulkanMemAlloc::~VulkanMemAlloc() {
    m_Device.waitIdle();
    vmaDestroyAllocator(m_Allocator);
}

