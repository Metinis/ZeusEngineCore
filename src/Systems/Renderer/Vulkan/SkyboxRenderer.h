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
        uint32_t getSkyboxReadIdx() const {return m_EqMap.readIdx;}
        uint32_t getIrradianceReadIdx() const {return m_IrradianceMap.readIdx;}
        uint32_t getPrefilterReadIdx() const {return m_PrefilterMap.readIdx;}
        uint32_t getBRDFReadIdx() const {return m_BRDFTex.readIdx;}
        void setDirty(const bool value) {m_IsDirty = value;}
        void cleanup();
        ~SkyboxRenderer();
    private:
        void initEqMapPipeline();
        void initIrradiancePipeline();
        void initPrefilterPipeline();
        void initBRDFPipeline();

        VKRenderer* m_Renderer{};

        //pipelines needed
        VkPipeline m_EqMapPipeline{};
        VkPipeline m_IrradianceMapPipeline{};
        VkPipeline m_PrefilterPipeline{};
        VkPipeline m_BRDFPipeline{};

        //images loaded/generated
        GPUTexture m_UploadedImage{};
        AllocatedImage m_EqMap{};
        AllocatedImage m_IrradianceMap{};
        AllocatedImage m_PrefilterMap{};
        AllocatedImage m_BRDFTex{};

        bool m_IsDirty{true};
    };
}
