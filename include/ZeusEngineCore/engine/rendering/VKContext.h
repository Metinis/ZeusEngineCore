#pragma once
#include <vulkan/vulkan.h>

namespace ZEN {
    class VKContext {
    public:
        VKContext();
        void init();
        void cleanup();
    private:
        void initVulkan();

        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugMessenger{};
        VkPhysicalDevice m_PhysicalDevice{};
        VkDevice m_Device{};
        VkSurfaceKHR m_Surface{};
        VkQueue m_GraphicsQueue{};
        uint32_t m_GraphicsQueueFamily{};
        bool m_Initialized{};
        friend class VKResources;
        friend class RenderContext;
        friend class SkyboxRenderer;
    };
}
