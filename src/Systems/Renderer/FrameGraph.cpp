#include "FrameGraph.h"

#include "Vulkan/VKInit.h"

using namespace ZEN;

static PhysicalResourceState translateUsageToPhysicalState(ResourceUsage usage) {
    PhysicalResourceState state;
    switch (usage) {
        case ResourceUsage::COLOR_ATTACHMENT_WRITE:
            state.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            state.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            state.accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case ResourceUsage::DEPTH_STENCIL_WRITE:
            state.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            state.stageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
            state.accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case ResourceUsage::SHADER_READ:
            state.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            state.stageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            state.accessMask = VK_ACCESS_2_SHADER_READ_BIT;
            break;
        case ResourceUsage::COMPUTE_READ_WRITE:
            state.layout = VK_IMAGE_LAYOUT_GENERAL;
            state.stageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            state.accessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
            break;
        case ResourceUsage::TRANSFER_DESTINATION:
            state.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            state.stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            state.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case ResourceUsage::TRANSFER_SOURCE:
            state.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            state.stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            state.accessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            break;
        default:
            spdlog::error("Fatal Error: Unrecognized resource usage mapping requested.");
    }
    return state;
}

FrameGraph::FrameGraph() {

}

void FrameGraph::compileAndExecute(VkCommandBuffer cmd) {
    for (auto &pass : m_RenderPasses) {
        std::vector<VkImageMemoryBarrier2> synthesizedBarriers{};

        auto checkRequirement = [&](const FrameResourceRequirement& req) {
            if (!m_Registry.contains(req.name)) {
                spdlog::error("Frame Graph: Could not find resource requirement for {}", req.name);
            }
            GraphResource& res = m_Registry[req.name];
            PhysicalResourceState requiredState = translateUsageToPhysicalState(req.usage);
            if (requiredState.layout != res.currentState.layout ||
            requiredState.accessMask != res.currentState.accessMask ||
            requiredState.stageMask != res.currentState.stageMask) {
                //need to do res -> req
                VkImageMemoryBarrier2 barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
                barrier.pNext = nullptr;
                barrier.srcStageMask = res.currentState.stageMask;
                barrier.srcAccessMask = res.currentState.accessMask;
                barrier.dstStageMask = requiredState.stageMask;
                barrier.dstAccessMask = requiredState.accessMask;

                barrier.oldLayout = res.currentState.layout;
                barrier.newLayout = requiredState.layout;

                VkImageAspectFlags aspectMask = (requiredState.layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                    requiredState.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) ?
                VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange = VKInit::imageSubresourceRange(aspectMask);
                barrier.image = res.physicalImage;
                synthesizedBarriers.push_back(barrier);
                res.currentState = requiredState;
                //spdlog::debug("Frame Graph: Barrier Synthesized for: {}", res.name);
            }
        };

        for (auto& read : pass.m_ReadResources) {
            checkRequirement(read);
        }

        for (auto& write : pass.m_WriteResources) {
            checkRequirement(write);
        }

        VkDependencyInfo dep{};
        dep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dep.pNext = nullptr;
        dep.imageMemoryBarrierCount = synthesizedBarriers.size();
        dep.pImageMemoryBarriers = synthesizedBarriers.data();

        vkCmdPipelineBarrier2(cmd, &dep);

        pass.execute(cmd);
    }
}


void FrameGraph::addRenderPass(RenderPass&& renderPass) {
    spdlog::debug("Frame graph: Pass added {}", renderPass.name);
    m_RenderPasses.push_back(std::move(renderPass));
}

void FrameGraph::removeRenderPass(std::string_view name) {
    for (size_t i{}; i < m_RenderPasses.size(); ++i) {
        if (m_RenderPasses[i].name == name) {
            m_RenderPasses.erase(m_RenderPasses.begin() + i);
        }
    }
}

void FrameGraph::registerDependency(const std::string& name, VkImage physicalImage) {
    GraphResource resource{};
    resource.name = name;
    resource.physicalImage = physicalImage;
    resource.currentState.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    resource.currentState.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    resource.currentState.accessMask = 0;
    m_Registry[name] = resource;
    //spdlog::debug("Frame graph: Registered {}", name);
}
