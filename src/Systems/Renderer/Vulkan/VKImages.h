#pragma once
#include <vulkan/vulkan.h>

#include "vma/vk_mem_alloc.h"

namespace ZEN {
    struct AllocatedImage {
        VkImage image{};
        VkImageView imageView{};
        VmaAllocation allocation{};
        VkExtent3D imageExtent{};
        VkFormat imageFormat{};
    };
    class VKImages {
    public:
        static void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        static void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize,
            VkExtent2D dstSize);
    };
}
