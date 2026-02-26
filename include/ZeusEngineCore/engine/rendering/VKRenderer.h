#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <VkBootstrap.h>

namespace ZEN {
    struct FrameData {
        VkCommandPool m_CommandPool{};
        VkCommandBuffer m_MainCommandBuffer{};
        VkSemaphore m_SwapChainSemaphore{}; //wait for SwapChain image request
        VkSemaphore m_RenderSemaphore{}; //presenting image to OS after drawing
        VkFence m_Fence{}; //wait for commands for a frame to finish
    };
    constexpr unsigned int FRAME_OVERLAP = 2;

    class VKRenderer {
    public:
        VKRenderer();
        void init();
        void draw();
        void cleanup();
        ~VKRenderer();
    private:
        void initVulkan();
        void initSwapChain();
        void initCommands();
        void initSyncStructures();

        void createSwapChain(uint32_t width, uint32_t height);
        void destroySwapChain();

        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugMessenger{};
        VkPhysicalDevice m_PhysicalDevice{};
        VkDevice m_Device{};
        VkSurfaceKHR m_Surface{};

        VkSwapchainKHR m_SwapChain{};
        VkFormat m_SwapChainImageFormat{};
        std::vector<VkImage> m_SwapChainImages{};
        std::vector<VkImageView> m_SwapChainImageViews{};
        VkExtent2D m_SwapChainExtent{};

        FrameData m_Frames[FRAME_OVERLAP];
        FrameData& getCurrentFrame() {return m_Frames[m_FrameNumber % FRAME_OVERLAP];}

        VkQueue m_GraphicsQueue{};
        uint32_t m_GraphicsQueueFamily{};

        uint64_t m_FrameNumber{};
        bool m_Initialized{};
    };
}