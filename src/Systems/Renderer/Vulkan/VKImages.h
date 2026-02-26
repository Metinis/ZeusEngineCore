#pragma once
#include <vulkan/vulkan.h>

namespace ZEN {
    class VKImages {
    public:
        static void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

    };
}
