#pragma once
#include <vulkan/vulkan.h>

namespace ZEN {
    enum class ResourceUsage {
        UNKNOWN,
        COLOR_ATTACHMENT_WRITE,
        DEPTH_STENCIL_WRITE,
        DEPTH_STENCIL_READ,
        SHADER_READ,
        COMPUTE_READ_WRITE,
        TRANSFER_DESTINATION,
        TRANSFER_SOURCE,
        PRESENT_SRC,
    };
    struct PhysicalResourceState {
        VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
        VkPipelineStageFlags2 stageMask{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
        VkAccessFlags2 accessMask{0};
    };
    struct GraphResource {
        std::string name{};
        VkImage physicalImage{VK_NULL_HANDLE};
        PhysicalResourceState currentState{};

        /*uint32_t width = 0;
        uint32_t height = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;*/
    };
    //represents a depencency
    struct FrameResourceRequirement {
        std::string name{};
        ResourceUsage usage{};
    };
    struct RenderPass {
        std::string name{};
        std::vector<FrameResourceRequirement> m_WriteResources{};
        std::vector<FrameResourceRequirement> m_ReadResources{};
        std::function<void(VkCommandBuffer)> execute{};
    };

    class FrameGraph {
    public:
        FrameGraph();

        void compileAndExecute(VkCommandBuffer cmd);
        void addRenderPass(RenderPass&& renderPass);
        void removeRenderPass(std::string_view name);
        void registerDependency(const std::string& name, VkImage physicalImage);
    private:
        std::unordered_map<std::string, GraphResource> m_Registry{};
        std::vector<RenderPass> m_RenderPasses{};
    };
}
