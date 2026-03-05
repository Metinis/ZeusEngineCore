#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "glm/mat4x4.hpp"

namespace ZEN {
    struct AllocatedBuffer {
        VkBuffer buffer{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocationInfo{};
    };
    struct GPUMeshBuffers {
        AllocatedBuffer indexBuffer{};
        AllocatedBuffer vertexBuffer{};
        VkDeviceAddress vertexBufferAddress{};
    };
    struct GPUDrawPushConstants {
        glm::mat4 worldMatrix{};
        VkDeviceAddress vertexBuffer{};
    };
}


