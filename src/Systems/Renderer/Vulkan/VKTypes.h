#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "VKImages.h"
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
        uint32_t indexCount{};
    };
    struct GPUTexture {
        AllocatedImage image{};
        VkSampler sampler{};
        uint32_t index{};
    };
    struct alignas(16) GPUMaterial {
        glm::vec4 u_Albedo{};   // xyz = color
        glm::vec4 u_Params{};   // x=metallic, y=roughness, z=ao, w=unused

        uint32_t albedoIndex{};
        uint32_t metallicIndex{};
        uint32_t roughnessIndex{};
        uint32_t normalIndex{};
        uint32_t aoIndex{};

        uint32_t idx;
        uint32_t pad0;
        uint32_t pad1;
    };
    struct alignas(16) GPUDrawPushConstants{
        uint32_t matIndex{1};
        glm::vec3 pad;
        glm::mat4 worldMatrix;
        VkDeviceAddress vertexBuffer;

    };
}


