#pragma once
#include <vulkan/vulkan.h>
#include "VKResources.h"
#include "Systems/Renderer/FrameGraph.h"
#include "Systems/Renderer/Vulkan/VKDescriptors.h"
#include "Systems/Renderer/Vulkan/VKTypes.h"
#include "Systems/Renderer/Vulkan/SkyboxRenderer.h"

namespace ZEN {
    class SkyboxRenderer;
    struct FrameData {
        VkCommandPool m_CommandPool{};
        VkCommandBuffer m_MainCommandBuffer{};
        VkSemaphore m_SwapChainSemaphore{}; //wait for SwapChain image request
        VkSemaphore m_RenderSemaphore{}; //presenting image to OS after drawing
        VkFence m_Fence{}; //wait for commands for a frame to finish
        DeletionQueue m_DeletionQueue{};
        DescriptorAllocatorGrowable m_FrameDescriptors;
        AllocatedBuffer m_IndirectBuffer;
        AllocatedBuffer m_ObjectBuffer;
        AllocatedBuffer m_SceneBuffer;
    };

    struct DrawCall {
        AssetID meshID;
        AssetID materialID;
        glm::mat4 model;
    };

    struct IndirectDrawCall {
        GPUMeshBuffers* mesh{};
        StoredMaterial* material{};
        int drawIndex{};
        int count{};
    };

    constexpr unsigned int FRAME_OVERLAP = 3;
    class RenderContext {
    public:
        RenderContext();
        void init(VKContext* stateCtx, VKResources* resourceCtx);
        void beginFrame();
        void draw();
        void endFrame();
        void drawImgui(VkCommandBuffer cmd, VkImageView targetImageView);
        void drawGeometry(VkCommandBuffer cmd);
        void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
        const FrameGraph& getFrameGraph() const {return m_FrameGraph;}
        std::vector<IndirectDrawCall> processDrawCalls();
        void cleanup();
    private:
        void initCommands();
        void initSyncStructures();
        void initFrameGraph();
        void initSwapChain();
        void createSwapChain(uint32_t width, uint32_t height);
        void recreateSwapChain();
        void destroySwapChain();
        void executeDrawCalls(VkCommandBuffer cmd, const std::vector<IndirectDrawCall>& draws);

        VKContext* m_StateCtx{};
        VKResources* m_ResourceCtx{};

        std::vector<DrawCall> m_DrawCalls{};

        FrameData m_Frames[FRAME_OVERLAP];
        FrameData& getCurrentFrame() {return m_Frames[m_FrameNumber % FRAME_OVERLAP];}
        uint64_t m_FrameNumber{0};
        VkFence m_ImmediateFence{};
        VkCommandBuffer m_ImmediateCommandBuffer{};
        VkCommandPool m_ImmediateCommandPool{};
        FrameGraph m_FrameGraph{};
        AllocatedImage m_DrawImage{};
        VkExtent2D m_DrawExtent{};
        AllocatedImage m_DepthImage{};

        VkSwapchainKHR m_SwapChain{};
        bool m_SwapChainRecreated{false};
        VkFormat m_SwapChainImageFormat{};
        std::vector<VkImage> m_SwapChainImages{};
        std::vector<VkImageView> m_SwapChainImageViews{};
        VkExtent2D m_SwapChainExtent{};
        bool m_RenderToIMGUITexture{true};

        std::unique_ptr<SkyboxRenderer> m_SkyboxRenderer;
        friend class SkyboxRenderer;
        friend class VKResources;
        friend class VKRenderer;
    };
}
