#pragma once
#include "vulkan/vulkan.h"

namespace ZEN::VKHelpers {
    inline VkSamplerCreateInfo getDefaultSamplerInfo() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0.0f;
        //samplerInfo.anisotropyEnable = VK_TRUE;
        //todo check this
        //samplerInfo.maxAnisotropy = 16.0f;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        return samplerInfo;
    }

    inline VkSamplerCreateInfo getCubeMapSamplerInfo() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        return samplerInfo;
    }

    // Linear filtered, clamp to edge
    inline VkSamplerCreateInfo linearClampedSampler() {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.anisotropyEnable = VK_FALSE;
        info.maxAnisotropy = 1.0f;
        info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.compareOp = VK_COMPARE_OP_ALWAYS;
        info.mipLodBias = 0.0f;
        info.minLod = 0.0f;
        info.maxLod = VK_LOD_CLAMP_NONE;
        return info;
    }

    // Nearest neighbor, clamp to edge
    inline VkSamplerCreateInfo nearestClampedSampler() {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_NEAREST;
        info.minFilter = VK_FILTER_NEAREST;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.anisotropyEnable = VK_FALSE;
        info.maxAnisotropy = 1.0f;
        info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.compareOp = VK_COMPARE_OP_ALWAYS;
        info.mipLodBias = 0.0f;
        info.minLod = 0.0f;
        info.maxLod = VK_LOD_CLAMP_NONE;
        return info;
    }

    // Linear filtered, repeat
    inline VkSamplerCreateInfo linearRepeatSampler() {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        //info.anisotropyEnable = VK_FALSE;
        //info.maxAnisotropy = 1.0f;
        info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.compareOp = VK_COMPARE_OP_ALWAYS;
        info.mipLodBias = 0.0f;
        info.minLod = 0.0f;
        info.maxLod = VK_LOD_CLAMP_NONE;
        return info;
    }

    // Nearest neighbor, repeat
    inline VkSamplerCreateInfo nearestRepeatSampler() {
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_NEAREST;
        info.minFilter = VK_FILTER_NEAREST;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.anisotropyEnable = VK_FALSE;
        info.maxAnisotropy = 1.0f;
        info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        info.unnormalizedCoordinates = VK_FALSE;
        info.compareEnable = VK_FALSE;
        info.compareOp = VK_COMPARE_OP_ALWAYS;
        info.mipLodBias = 0.0f;
        info.minLod = 0.0f;
        info.maxLod = VK_LOD_CLAMP_NONE;
        return info;
    }

    inline VkSamplerCreateInfo toVkSamplerCreateInfo(const SamplerInfo &info) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        samplerInfo.magFilter = info.magFilter;
        samplerInfo.minFilter = info.minFilter;

        samplerInfo.mipmapMode = info.mipmapMode;

        samplerInfo.addressModeU = info.addressModeU;
        samplerInfo.addressModeV = info.addressModeV;
        samplerInfo.addressModeW = info.addressModeW;

        samplerInfo.mipLodBias = info.mipLodBias;

        samplerInfo.anisotropyEnable =
                info.anisotropyEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = info.maxAnisotropy;

        samplerInfo.compareEnable =
                info.compareEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.compareOp = info.compareOp;

        samplerInfo.minLod = info.minLod;
        samplerInfo.maxLod = info.maxLod;

        samplerInfo.borderColor = info.borderColor;

        samplerInfo.unnormalizedCoordinates =
                info.unnormalizedCoordinates ? VK_TRUE : VK_FALSE;

        return samplerInfo;
    }

    inline SamplerInfo fromVkSamplerCreateInfo(const VkSamplerCreateInfo &vk) {
        SamplerInfo info{};

        info.magFilter = vk.magFilter;
        info.minFilter = vk.minFilter;

        info.mipmapMode = vk.mipmapMode;

        info.addressModeU = vk.addressModeU;
        info.addressModeV = vk.addressModeV;
        info.addressModeW = vk.addressModeW;

        info.mipLodBias = vk.mipLodBias;

        info.anisotropyEnable = (vk.anisotropyEnable == VK_TRUE);
        info.maxAnisotropy = vk.maxAnisotropy;

        info.compareEnable = (vk.compareEnable == VK_TRUE);
        info.compareOp = vk.compareOp;

        info.minLod = vk.minLod;
        info.maxLod = vk.maxLod;

        info.borderColor = vk.borderColor;

        info.unnormalizedCoordinates = (vk.unnormalizedCoordinates == VK_TRUE);

        return info;
    }
}
