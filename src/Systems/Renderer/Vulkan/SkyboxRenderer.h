#pragma once
#include <vulkan/vulkan.h>
#include "VKImages.h"
#include "VKTypes.h"

namespace ZEN {
    class VKRenderer;
    class SkyboxRenderer {
    public:
        SkyboxRenderer(VKRenderer* renderer);
        void init(std::filesystem::path const &path);
        void render(VkCommandBuffer cmd);
        void setDirty(const bool value) {m_IsDirty = value;}
        void cleanup();
        ~SkyboxRenderer();
    private:
        void initComputePipeline();

        VKRenderer* m_Renderer{};

        //pipelines needed
        VkPipelineLayout m_SkyboxPipelineLayout{};
        VkPipeline m_EqMapPipeline{};

        //images loaded/generated
        GPUTexture m_UploadedImage{};
        AllocatedImage m_EqMap{};

        bool m_IsDirty{true};

        //todo temp, just give the cubemap idx
        friend class VKRenderer;
    };
}
