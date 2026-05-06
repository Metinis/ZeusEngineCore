#include "FrameGraph.h"

#include "Vulkan/VKInit.h"

using namespace ZEN;

static std::vector<int> topoSort(std::vector<std::vector<int>>& adj) {
    int n = adj.size();
    std::vector<int> indegree(n, 0);
    std::queue<int> q;
    std::vector<int> list;

    //Compute indegrees
    for (int i = 0; i < n; i++) {
        for (int next : adj[i])
            indegree[next]++;
    }
    //Add all nodes with indegree 0
    for (int i = 0; i < n; i++)
        if (indegree[i] == 0)
            q.push(i);

    //Kahn’s Algorithm
    while (!q.empty()) {
        int top = q.front();
        q.pop();
        list.push_back(top);
        for (int next : adj[top]) {
            indegree[next]--;
            if (indegree[next] == 0)
                q.push(next);
        }
    }
    return list;
}

static std::vector<std::vector<int>> buildAdjacencylist(std::vector<RenderPass>& passes) {
    std::vector<std::vector<int>> adjList(passes.size());
    std::unordered_map<std::string, int> lastWriter;
    //find all writer -> reader relationships
    for (int i = 0; i < passes.size(); ++i) {
        auto& pass = passes[i];

        for (auto& writeResource : pass.m_WriteResources) {
            //If there was a previous writer, add dependency from previous writer to this pass
            if (lastWriter.contains(writeResource.name)) {
                int prevWriter = lastWriter[writeResource.name];
                adjList[prevWriter].push_back(i);
            }
            //Update the last writer for this resource
            lastWriter[writeResource.name] = i;
        }
    }

    //add dependencies for readers (if a resource is read, it must come after the writer)
    for (int i = 0; i < passes.size(); ++i) {
        auto& pass = passes[i];

        for (auto& readResource : pass.m_ReadResources) {
            // Find the most recent writer of this resource
            if (lastWriter.contains(readResource.name)) {
                int writerIdx = lastWriter[readResource.name];
                //Dont add self-dependency
                if (writerIdx != i) {
                    adjList[writerIdx].push_back(i);
                }
            }
        }
    }
    return adjList;
}

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
        case ResourceUsage::DEPTH_STENCIL_READ:
            state.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            state.stageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
            state.accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
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
        case ResourceUsage::PRESENT_SRC:
            state.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            state.stageMask = VK_PIPELINE_STAGE_2_NONE;
            state.accessMask = VK_ACCESS_2_NONE;
            break;
        default:
            spdlog::error("Fatal Error: Unrecognized resource usage mapping requested.");
    }
    return state;
}

FrameGraph::FrameGraph() {

}

void FrameGraph::compileAndExecute(VkCommandBuffer cmd) {

    std::vector<std::vector<int>> adjList = buildAdjacencylist(m_RenderPasses);
    std::vector<int> sorted = topoSort(adjList);

    for (int idx : sorted) {
        auto &pass = m_RenderPasses[idx];

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
                for (auto &resource: m_Registry | std::views::values) {
                    if (resource.physicalImage == res.physicalImage) {
                        resource.currentState = requiredState;
                    }
                }
                //res.currentState = requiredState;

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
    resource.currentState.stageMask = VK_PIPELINE_STAGE_2_NONE;
    resource.currentState.accessMask = VK_ACCESS_2_NONE;
    m_Registry[name] = resource;
    //spdlog::debug("Frame graph: Registered {}", name);
}
