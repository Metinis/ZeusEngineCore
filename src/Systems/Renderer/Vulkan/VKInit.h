#pragma once
#include "vulkan/vulkan.h"

namespace ZEN {
    class VKInit {
    public:
        static VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
        static VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count);
        static VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);
        static VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
        static VkCommandBufferBeginInfo cmdBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
        static VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask);
        static VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
        static VkCommandBufferSubmitInfo cmdBufferSubmitInfo(VkCommandBuffer cmd);
        static VkSubmitInfo2 submitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo,
            VkSemaphoreSubmitInfo* waitSemaphoreInfo);
    };
}
