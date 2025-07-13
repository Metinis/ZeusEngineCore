#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include "ZeusEngineCore/ScopedWaiter.h"
#include "Buffer.h"

namespace ZEN::VKAPI {
    class MemAllocator {
    public:
        MemAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice);

        ~MemAllocator();

        VmaAllocator Get() const { return m_Allocator; }

    private:
        VmaAllocator m_Allocator;
        vk::Device m_Device;
    };
}