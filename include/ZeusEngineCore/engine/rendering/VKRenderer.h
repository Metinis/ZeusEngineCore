#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <VkBootstrap.h>

namespace ZEN {
    class VKRenderer {
    public:
        VKRenderer();
        void init();
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

        bool m_Initialized{};
    };
}