#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "VKImages.h"
#include "glm/mat4x4.hpp"
#include "ZeusEngineCore/asset/AssetTypes.h"

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
        TextureType type{Texture2D};
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
    struct GPUDrawPushConstants {
        int i;
    };

}

struct VkSamplerCreateInfoEqual
{
    bool operator()(
        const VkSamplerCreateInfo& a,
        const VkSamplerCreateInfo& b
    ) const
    {
        return
            a.flags == b.flags &&
            a.magFilter == b.magFilter &&
            a.minFilter == b.minFilter &&
            a.mipmapMode == b.mipmapMode &&
            a.addressModeU == b.addressModeU &&
            a.addressModeV == b.addressModeV &&
            a.addressModeW == b.addressModeW &&
            a.mipLodBias == b.mipLodBias &&
            a.anisotropyEnable == b.anisotropyEnable &&
            a.maxAnisotropy == b.maxAnisotropy &&
            a.compareEnable == b.compareEnable &&
            a.compareOp == b.compareOp &&
            a.minLod == b.minLod &&
            a.maxLod == b.maxLod &&
            a.borderColor == b.borderColor &&
            a.unnormalizedCoordinates == b.unnormalizedCoordinates;
    }
};

namespace std {
    template<>
    struct hash<VkSamplerCreateInfo>
    {
        size_t operator()(const VkSamplerCreateInfo& info) const
        {
            size_t seed = 0;

            auto hashCombine = [&](size_t value)
            {
                seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };

            hashCombine(std::hash<uint32_t>()(info.flags));

            hashCombine(std::hash<int>()(info.magFilter));
            hashCombine(std::hash<int>()(info.minFilter));

            hashCombine(std::hash<int>()(info.mipmapMode));

            hashCombine(std::hash<int>()(info.addressModeU));
            hashCombine(std::hash<int>()(info.addressModeV));
            hashCombine(std::hash<int>()(info.addressModeW));

            hashCombine(std::hash<float>()(info.mipLodBias));

            hashCombine(std::hash<uint32_t>()(info.anisotropyEnable));
            hashCombine(std::hash<float>()(info.maxAnisotropy));

            hashCombine(std::hash<uint32_t>()(info.compareEnable));
            hashCombine(std::hash<int>()(info.compareOp));

            hashCombine(std::hash<float>()(info.minLod));
            hashCombine(std::hash<float>()(info.maxLod));

            hashCombine(std::hash<int>()(info.borderColor));

            hashCombine(std::hash<uint32_t>()(info.unnormalizedCoordinates));

            return seed;
        }
    };
}


