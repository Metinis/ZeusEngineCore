#pragma once
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include "imgui_impl_vulkan.h"
#include "Systems/Renderer/FrameGraph.h"
#include "Systems/Renderer/Vulkan/VKDescriptors.h"
#include "Systems/Renderer/Vulkan/VKImages.h"
#include "Systems/Renderer/Vulkan/VKPipelines.h"
#include "Systems/Renderer/Vulkan/VKTypes.h"
#include "ZeusEngineCore/asset/AssetTypes.h"

namespace ZEN {
    class SkyboxRenderer;
    class Scene;
    class CameraSystem;
    struct EngineContext;
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
        AllocatedBuffer m_IndirectBuffer;
        AllocatedBuffer m_ObjectBuffer;
        AllocatedBuffer m_SceneBuffer;
    };
    struct GPUSceneData {
        glm::mat4 proj;
        glm::mat4 view;
        glm::vec4 ambientColor;
        glm::vec4 sunlightDirection; // w for sun power
        glm::vec4 sunlightColor;
        glm::vec4 cameraPosition;
    };
    constexpr unsigned int FRAME_OVERLAP = 3;

    struct IndexAllocator {
        std::vector<uint32_t> availableList{};
        std::vector<uint32_t> freeList{};

        uint32_t allocate();
        uint32_t allocate(uint32_t idx);
        void free(uint32_t idx);
        void init(uint32_t max);
        void flush();
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

    struct VKContext {
        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugMessenger{};
        VkPhysicalDevice m_PhysicalDevice{};
        VkDevice m_Device{};
        VkSurfaceKHR m_Surface{};
        VkQueue m_GraphicsQueue{};
        uint32_t m_GraphicsQueueFamily{};
        bool m_Initialized{};
    };

    struct SwapchainContext {
        VkSwapchainKHR m_SwapChain{};
        bool m_SwapChainRecreated{false};
        VkFormat m_SwapChainImageFormat{};
        std::vector<VkImage> m_SwapChainImages{};
        std::vector<VkImageView> m_SwapChainImageViews{};
        VkExtent2D m_SwapChainExtent{};
    };

    struct RenderContext {
        FrameData m_Frames[FRAME_OVERLAP];
        FrameData& getCurrentFrame() {return m_Frames[m_FrameNumber % FRAME_OVERLAP];}
        uint64_t m_FrameNumber{0};
        VkFence m_ImmediateFence{};
        VkCommandBuffer m_ImmediateCommandBuffer{};
        VkCommandPool m_ImmediateCommandPool{};
        FrameGraph m_FrameGraph{};
        SwapchainContext m_SwapchainCtx{};
        AllocatedImage m_DrawImage{};
        VkExtent2D m_DrawExtent{};
        AllocatedImage m_DepthImage{};
        std::vector<DrawCall> m_DrawCalls{};
        bool m_RenderToIMGUITexture{true};
    };

    struct ResourceContext {
        VmaAllocator m_Allocator{};

        DescriptorAllocator m_GlobalDescriptorAllocator{};
        DescriptorAllocator m_TextureDescriptorAllocator{};
        VkDescriptorSet m_TextureDescriptorSet{};
        VkDescriptorSetLayout m_TextureDescriptorSetLayout{};

        VkDescriptorSet m_MaterialDescriptorSet{};
        VkDescriptorSetLayout m_MaterialDescriptorSetLayout{};

        VkDescriptorSet m_DrawImageDescriptors{};
        VkDescriptorSetLayout m_DrawImageDescriptorLayout{};

        VkDescriptorSet m_ImGuiDescriptorSet{}; //used for color image
        VkDescriptorSet m_ImGUIErrorSet{}; //no texture found
        std::unordered_map<AssetID, VkDescriptorSet> m_ImGUIDescSetMap{}; //used for thumbnails since imgui doesnt support bindless

        VkPipelineLayout m_MainPipelineLayout{};
        VkPipelineLayout m_ComputePipelineLayout{};
        //todo access this within the pipeline cache

        GPUTexture m_ErrorTexture{};

        GPUSceneData m_SceneData{};
        VkDescriptorSetLayout m_FrameDescriptorLayout{};

        AllocatedBuffer m_MaterialBuffer{};

        std::unordered_map<AssetID, GPUMeshBuffers> m_MeshMap{};
        std::unordered_map<AssetID, StoredTexture> m_TextureMap{}; //texture and index into bindless
        std::unordered_map<AssetID, StoredMaterial> m_MaterialMap{}; //material and index into material buff
        //hashed infos todo: ref count delete
        //todo need compute working
        std::unordered_map<PipelineInfo, VkPipeline> m_PipelineMap{};
        std::unordered_map<VkSamplerCreateInfo, StoredSampler, std::hash<VkSamplerCreateInfo>, VkSamplerCreateInfoEqual> m_SamplerMap{};

        IndexAllocator m_TextureAllocator{};
        IndexAllocator m_StorageImageAllocator{};
        IndexAllocator m_MaterialAllocator{};
        IndexAllocator m_SamplerAllocator{};

        DeletionQueue m_DeletionQueue{};
    };

    class ZEN_API VKRenderer {
    public:
        VKRenderer();
        void init(EngineContext* ctx);
        void initFrameGraph();
        void beginFrame();
        void draw();
        void endFrame();
        void submitDrawCall(const DrawCall& call);
        void executeDrawCalls(VkCommandBuffer cmd, const std::vector<IndirectDrawCall>& draws);
        void setImGUIMode(const bool mode);
        StoredSampler getSampler(const VkSamplerCreateInfo& info);
        VkDescriptorSet getImGUIDescSet(AssetID id);
        //will create mapping between assetID and GPU mesh to be used by renderer
        GPUMeshBuffers uploadMesh(AssetID id, const MeshData& mesh);
        GPUTexture uploadTexture(AssetID id, const TextureData& texture);
        GPUMaterial uploadMaterial(AssetID id, const Material& material);
        void deleteMesh(AssetID id);
        void deleteTexture(AssetID id);
        void deleteMaterial(AssetID id);
        const FrameGraph& getFrameGraph() {return m_RenderCtx.m_FrameGraph;}
        //void createMesh()
        ImGui_ImplVulkan_InitInfo initImgui();
        [[nodiscard]] VkDescriptorSet getImDescSet() const {return m_ResourceCtx.m_ImGuiDescriptorSet;}
        void drawImgui(VkCommandBuffer cmd, VkImageView targetImageView);
        void drawGeometry(VkCommandBuffer);
        void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
        AllocatedImage createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false, int layers = 1);
        AllocatedImage createImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false, int layers = 1);
        void destroyImage(const AllocatedImage& img);
        void cleanup();
        ~VKRenderer();
    private:
        void initVulkan();
        void initSwapChain();
        void initCommands();
        void initSyncStructures();
        void initDescriptors();
        void initPipelines();
        void initErrorTexture();
        void initMainSamplers();

        void initMainPipeLayout();
        void initMainComputeLayout();
        VkPipeline createMainPipeline(const PipelineInfo& pipelineInfo);
        void generateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent3D size,
            uint32_t mipLevels, uint32_t layerCount);
        std::vector<IndirectDrawCall> processDrawCalls();
        void prepareDescriptors(VkCommandBuffer cmd);

        void createSwapChain(uint32_t width, uint32_t height);
        void recreateSwapChain();
        void destroySwapChain();

        AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void destroyBuffer(const AllocatedBuffer& buffer);

        VKContext m_StateCtx{};
        RenderContext m_RenderCtx{};
        ResourceContext m_ResourceCtx{};

        Scene* m_Scene{};
        CameraSystem* m_CameraSystem{};

        //todo just pass a context struct rather than expose everything
        std::unique_ptr<SkyboxRenderer> m_SkyboxRenderer{};
        friend class SkyboxRenderer;
    };
}
