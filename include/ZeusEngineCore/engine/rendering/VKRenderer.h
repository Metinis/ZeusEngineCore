#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>

#include "../../../../src/Systems/Renderer/Vulkan/VKDescriptors.h"
#include "../../../../src/Systems/Renderer/Vulkan/VKImages.h"


namespace ZEN {
    struct DeletionQueue {
        std::deque<std::function<void()>> deletors{};

        void pushFunction(std::function<void()>&& function) {
            deletors.push_back(function);
        }

        void flush() {
            for (auto& deletor : std::ranges::reverse_view(deletors)) {
                deletor();
            }
            deletors.clear();
        }
    };
    struct FrameData {
        VkCommandPool m_CommandPool{};
        VkCommandBuffer m_MainCommandBuffer{};
        VkSemaphore m_SwapChainSemaphore{}; //wait for SwapChain image request
        VkSemaphore m_RenderSemaphore{}; //presenting image to OS after drawing
        VkFence m_Fence{}; //wait for commands for a frame to finish
        DeletionQueue m_DeletionQueue{};
    };
    constexpr unsigned int FRAME_OVERLAP = 3;

    class VKRenderer {
    public:
        VKRenderer();
        void init();
        void draw();
        void drawBackground(VkCommandBuffer cmd);
        void cleanup();
        ~VKRenderer();
    private:
        void initVulkan();
        void initSwapChain();
        void initCommands();
        void initSyncStructures();
        void initDescriptors();
        void initPipelines();
        void initBackgroundPipeline();

        void createSwapChain(uint32_t width, uint32_t height);
        void destroySwapChain();

        DeletionQueue m_DeletionQueue{};

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

        uint64_t m_FrameNumber{0};

        VmaAllocator m_Allocator{};

        AllocatedImage m_DrawImage{};
        VkExtent2D m_DrawExtent{};

        DescriptorAllocator m_GlobalDescriptorAllocator{};
        VkDescriptorSet m_DrawImageDescriptors{};
        VkDescriptorSetLayout m_DrawImageDescriptorLayout{};

        VkPipeline m_GradientPipeline{};
        VkPipelineLayout m_GradientPipelineLayout{};

        bool m_Initialized{};
    };
}
