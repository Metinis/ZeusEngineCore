#pragma once
#include "imgui_impl_vulkan.h"
#include "Systems/Renderer/Vulkan/VKDescriptors.h"
#include "Systems/Renderer/Vulkan/VKTypes.h"

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
    struct GPUSceneData {
        glm::mat4 proj;
        glm::mat4 view;
        glm::vec4 ambientColor;
        glm::vec4 sunlightDirection; // w for sun power
        glm::vec4 sunlightColor;
        glm::vec4 cameraPosition;
    };
    struct IndexAllocator {
        std::vector<uint32_t> availableList{};
        std::vector<uint32_t> freeList{};

        uint32_t allocate();
        uint32_t allocate(uint32_t idx);
        void free(uint32_t idx);
        void init(uint32_t max);
        void flush();
    };

    struct VKContext;
    struct RenderContext;

    class VKResources {
    public:
        VKResources();
        void init(VKContext* stateCtx, RenderContext* renderCtx);
        StoredSampler getSampler(const VkSamplerCreateInfo& info);
        VkDescriptorSet getImGUIDescSet(AssetID id);
        VkDescriptorSet getImDescSet() const {return m_ImGuiDescriptorSet;}
        GPUMeshBuffers uploadMesh(AssetID id, const MeshData& mesh);
        GPUTexture uploadTexture(AssetID id, const TextureData& texture);
        GPUMaterial uploadMaterial(AssetID id, const Material& material);
        AllocatedImage createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false, int layers = 1);
        AllocatedImage createImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false, int layers = 1);
        AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

        void deleteMesh(AssetID id);
        void deleteTexture(AssetID id);
        void deleteMaterial(AssetID id);
        void destroyImage(const AllocatedImage& img);
        void destroyBuffer(const AllocatedBuffer& buffer);

    private:
        void initAllocator(VKContext* stateCtx);
        void initDescriptors();
        void initPipelines();
        void initErrorTexture();
        void initMainSamplers();
        void initMainPipeLayout();
        void initMainComputeLayout();
        ImGui_ImplVulkan_InitInfo initImgui();
        void cleanup();

        VkPipeline createMainPipeline(const PipelineInfo& pipelineInfo);
        void generateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent3D size,
            uint32_t mipLevels, uint32_t layerCount);
        void prepareDescriptors(VkCommandBuffer cmd);

        VKContext* m_StateCtx{};
        RenderContext* m_RenderCtx{};

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
        friend class VKRenderer;
        friend class RenderContext;
        friend class SkyboxRenderer;

    };
}
