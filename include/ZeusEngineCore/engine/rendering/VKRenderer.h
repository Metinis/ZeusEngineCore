#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>

#include "imgui_impl_vulkan.h"
#include "../../../../src/Systems/Renderer/Vulkan/VKDescriptors.h"
#include "../../../../src/Systems/Renderer/Vulkan/VKImages.h"
#include "../../../../src/Systems/Renderer/Vulkan/VKTypes.h"
#include "ZeusEngineCore/asset/AssetTypes.h"



namespace ZEN {
    struct MeshData;

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
        DescriptorAllocatorGrowable m_FrameDescriptors;
    };
    struct GPUSceneData {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 viewproj;
        glm::vec4 ambientColor;
        glm::vec4 sunlightDirection; // w for sun power
        glm::vec4 sunlightColor;
    };
    constexpr unsigned int FRAME_OVERLAP = 3;

    class VKRenderer {
    public:
        VKRenderer();
        void init();
        void beginFrame();
        void draw();
        void endFrame();
        //will create mapping between assetID and GPU mesh to be used by renderer
        GPUMeshBuffers uploadMesh(AssetID id, const MeshData& mesh);
        void deleteMesh(AssetID id);
        //void createMesh()
        ImGui_ImplVulkan_InitInfo initImgui();
        [[nodiscard]] VkDescriptorSet getImDescSet() const {return m_ImGuiDescriptorSet;}
        void drawBackground(VkCommandBuffer cmd);
        void drawImgui(VkCommandBuffer cmd, VkImageView targetImageView);
        void drawGeometry(VkCommandBuffer);
        void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
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
        void initSampler();
        void initMeshPipeline();

        void createSwapChain(uint32_t width, uint32_t height);
        void recreateSwapChain();
        void destroySwapChain();

        AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void destroyBuffer(const AllocatedBuffer& buffer);

        DeletionQueue m_DeletionQueue{};

        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugMessenger{};
        VkPhysicalDevice m_PhysicalDevice{};
        VkDevice m_Device{};
        VkSurfaceKHR m_Surface{};

        VkSwapchainKHR m_SwapChain{};
        bool m_SwapChainRecreated{false};
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
        AllocatedImage m_DepthImage{};

        DescriptorAllocator m_GlobalDescriptorAllocator{};
        VkDescriptorSet m_DrawImageDescriptors{};
        VkDescriptorSetLayout m_DrawImageDescriptorLayout{};

        VkPipeline m_GradientPipeline{};
        VkPipelineLayout m_GradientPipelineLayout{};

        VkFence m_ImmediateFence{};
        VkCommandBuffer m_ImmediateCommandBuffer{};
        VkCommandPool m_ImmediateCommandPool{};

        VkDescriptorSet m_ImGuiDescriptorSet{};
        VkSampler m_Sampler{};

        VkPipelineLayout m_MeshPipelineLayout{};
        VkPipeline m_MeshPipeline{};

        GPUSceneData m_SceneData{};
        VkDescriptorSetLayout m_GpuSceneDataDescriptorLayout{};

        std::unordered_map<AssetID, GPUMeshBuffers> m_MeshMap{};

        bool m_Initialized{};
    };
}
