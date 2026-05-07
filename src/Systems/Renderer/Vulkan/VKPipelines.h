#pragma once
#include <vulkan/vulkan.h>

namespace ZEN {
    struct PipelineInfo {
        std::string vertexShader{"/shaders/vulkan-shaders/testTriangle.vert.spv"};
        std::string fragmentShader{"/shaders/vulkan-shaders/testTriangle.frag.spv"};
        VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
        VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};
        VkCullModeFlags cullMode{VK_CULL_MODE_BACK_BIT};
        VkFrontFace frontFace{VK_FRONT_FACE_COUNTER_CLOCKWISE};
        VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};
        bool multisamplingEnabled{false};
        bool blendingEnabled{false};
        bool blendingAdditive{false};
        bool blendingAlpha{false};
        //VkFormat colorAttachmentFormat{VK_FORMAT_B8G8R8A8_UNORM};
        //VkFormat depthFormat{VK_FORMAT_D32_SFLOAT};
        bool depthTestEnabled{true};
        bool depthWriteEnabled{true};
        VkCompareOp depthCompareOp{VK_COMPARE_OP_GREATER_OR_EQUAL};

        bool operator==(const PipelineInfo& other) const {
            return vertexShader == other.vertexShader &&
                   fragmentShader == other.fragmentShader &&
                   topology == other.topology &&
                   polygonMode == other.polygonMode &&
                   cullMode == other.cullMode &&
                   frontFace == other.frontFace &&
                   samples == other.samples &&
                   multisamplingEnabled == other.multisamplingEnabled &&
                   blendingEnabled == other.blendingEnabled &&
                   blendingAdditive == other.blendingAdditive &&
                   blendingAlpha == other.blendingAlpha &&
                   //colorAttachmentFormat == other.colorAttachmentFormat &&
                   //depthFormat == other.depthFormat &&
                   depthTestEnabled == other.depthTestEnabled &&
                   depthWriteEnabled == other.depthWriteEnabled &&
                   depthCompareOp == other.depthCompareOp;
        }
    };

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
        void enableBlendingAdditive();
        void enableBlendingAlpha();
        void setColorAttachmentFormat(VkFormat colorAttachmentFormat);
        void setDepthFormat(VkFormat format);
        void disableDepthTest();
        void enableDepthTest(bool depthWriteEnable, VkCompareOp op);
    };
}
namespace std {
    template<>
    struct hash<ZEN::PipelineInfo> {
        size_t operator()(const ZEN::PipelineInfo& info) const {
            size_t h = 0;

            h ^= hash<string>()(info.vertexShader) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<string>()(info.fragmentShader) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<int>()(static_cast<int>(info.topology)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<int>()(static_cast<int>(info.polygonMode)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<uint32_t>()(info.cullMode) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<int>()(static_cast<int>(info.frontFace)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<int>()(static_cast<int>(info.samples)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<bool>()(info.multisamplingEnabled) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<bool>()(info.blendingEnabled) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<bool>()(info.blendingAdditive) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<bool>()(info.blendingAlpha) + 0x9e3779b9 + (h << 6) + (h >> 2);
            //h ^= hash<int>()(static_cast<int>(info.colorAttachmentFormat)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            //h ^= hash<int>()(static_cast<int>(info.depthFormat)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<bool>()(info.depthTestEnabled) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<bool>()(info.depthWriteEnabled) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= hash<int>()(static_cast<int>(info.depthCompareOp)) + 0x9e3779b9 + (h << 6) + (h >> 2);

            return h;
        }
    };
}

