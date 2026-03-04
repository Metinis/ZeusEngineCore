#pragma once
#include <vulkan/vulkan.h>

namespace ZEN {
    class VKPipelines {
    public:
        static bool loadShaderModule(std::string_view filePath, VkDevice device, VkShaderModule* outShaderModule);
    };
    class VKPipelineBuilder {
    public:
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineMultisampleStateCreateInfo multiSampling{};
        VkPipelineLayout pipelineLayout{};
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        VkPipelineRenderingCreateInfo renderInfo{};
        VkFormat colorAttachmentFormat{};

        VKPipelineBuilder(){ clear(); }
        void clear();
        VkPipeline buildPipeline(VkDevice device);
        void setShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
        void setInputTopology(VkPrimitiveTopology topology);
        void setPolygonMode(VkPolygonMode polygonMode);
        void setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
        void setMultiSamplingNone();
        void disableBlending();
        void setColorAttachmentFormat(VkFormat colorAttachmentFormat);
        void setDepthFormat(VkFormat format);
        void disableDepthTest();
    };
}

