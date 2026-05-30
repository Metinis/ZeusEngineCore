#pragma once
#include <vulkan/vulkan.h>

#include "vma/vk_mem_alloc.h"

namespace ZEN {
    struct AllocatedImage {
        VkImage image{};

        //Full image view
        VkImageView imageView{};

        //One view per mip level
        std::vector<VkImageView> mipViews{};

        VmaAllocation allocation{};
        VkExtent3D imageExtent{};
        VkFormat imageFormat{};

        uint32_t mipLevels{};

        uint32_t readIdx{};

        std::vector<uint32_t> writeIdx{};
    };
    class VKImages {
    public:
        static void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
        static void transitionImage(
    VkCommandBuffer cmd,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkPipelineStageFlags2 srcStage,
    VkPipelineStageFlags2 dstStage,
    VkAccessFlags2 srcAccess,
    VkAccessFlags2 dstAccess);

        static void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize,
            VkExtent2D dstSize);
    };
}
