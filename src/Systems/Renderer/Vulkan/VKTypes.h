#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "VKImages.h"
#include "glm/mat4x4.hpp"

namespace ZEN {
    enum MaterialFlags : uint32_t {
        USE_ALBEDO   = 1 << 0,
        USE_METALLIC = 1 << 1,
        USE_ROUGHNESS= 1 << 2,
        USE_NORMAL   = 1 << 3,
        USE_AO       = 1 << 4
    };
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
    };
    struct StoredTexture {
        GPUTexture texture{};
        uint32_t idx{};
    };
    struct alignas(16) GPUMaterial {
        glm::vec4 u_Albedo{};   // xyz = color
        glm::vec4 u_Params{};   // x=metallic, y=roughness, z=ao, w=unused

        uint32_t albedoIndex{};
        uint32_t metallicIndex{};
        uint32_t roughnessIndex{};
        uint32_t normalIndex{};
        uint32_t aoIndex{};

        uint32_t flags{};
    };
    struct StoredMaterial {
        GPUMaterial material{};
        VkPipeline pipeline{};
        bool useDepth{true};
        uint32_t idx{};
    };
    struct alignas(16) GPUObjectData {
        uint32_t matIndex;
        glm::vec3 pad;
        glm::mat4 model;
        VkDeviceAddress vertexBuffer;
    };

    struct GPUIndirectCommand {
        VkDrawIndexedIndirectCommand draw;
        uint32_t objectIndex;
    };

    struct alignas(16) GPUDrawPushConstants{

    };
}


