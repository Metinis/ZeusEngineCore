#include "ZeusEngineCore/engine/rendering/VKBackend.h"
#include <vulkan/vk_enum_string_helper.h>
#include "SkyboxRenderer.h"
#include "VKInit.h"
#include "ZeusEngineCore/engine/rendering/VKUtils.h"
#include "GLFW/glfw3.h"
#include "ZeusEngineCore/core/Application.h"
#include "ZeusEngineCore/engine/rendering/VKContext.h"
#include <VkBootstrap.h>

using namespace ZEN;


static uint32_t swapChainImageIndex{}; //todo move this into member

RenderContext::RenderContext() {
}

void RenderContext::init(VKContext *stateCtx, VKResources *resourceCtx) {
    m_StateCtx = stateCtx;
    m_ResourceCtx = resourceCtx;
    m_SkyboxRenderer = std::make_unique<SkyboxRenderer>(m_StateCtx, m_ResourceCtx);
    m_ResourceCtx->initAllocator(stateCtx);
    initSwapChain();
    initCommands();
    initSyncStructures();
    initFrameGraph();

}

void RenderContext::initCommands() {
    VkCommandPoolCreateInfo commandPoolCreateInfo = VKInit::commandPoolCreateInfo(m_StateCtx->m_GraphicsQueueFamily,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (size_t i{}; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateCommandPool(m_StateCtx->m_Device, &commandPoolCreateInfo, nullptr, &m_Frames[i].m_CommandPool));

        VkCommandBufferAllocateInfo cmdAllocInfo = VKInit::commandBufferAllocateInfo(m_Frames[i].m_CommandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(m_StateCtx->m_Device, &cmdAllocInfo, &m_Frames[i].m_MainCommandBuffer));
    }

    //immediate submit
    VK_CHECK(vkCreateCommandPool(m_StateCtx->m_Device, &commandPoolCreateInfo, nullptr, &m_ImmediateCommandPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = VKInit::commandBufferAllocateInfo(m_ImmediateCommandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(m_StateCtx->m_Device, &cmdAllocInfo, &m_ImmediateCommandBuffer));

    m_ResourceCtx->m_DeletionQueue.pushFunction([=]() {
    vkDestroyCommandPool(m_StateCtx->m_Device, m_ImmediateCommandPool, nullptr);
    });
    spdlog::debug("Renderer: Initialized Commands");
}

void RenderContext::initFrameGraph() {
    RenderPass computePass{.name = "computePass"};
    computePass.m_WriteResources.push_back({"DrawImageCompute", ResourceUsage::COMPUTE_READ_WRITE});
    computePass.execute = [this](VkCommandBuffer cmd) {
        m_SkyboxRenderer->render(cmd);
    };
    RenderPass geometryPass{.name = "geometryPass"};
    geometryPass.m_ReadResources.push_back({"DrawImageCompute", ResourceUsage::SHADER_READ});
    geometryPass.m_WriteResources.push_back({"DrawImageGeometry", ResourceUsage::COLOR_ATTACHMENT_WRITE});
    geometryPass.m_WriteResources.push_back({"DepthImage", ResourceUsage::DEPTH_STENCIL_WRITE});
    geometryPass.execute = [this](VkCommandBuffer cmd) {
        drawGeometry(cmd);
    };
    RenderPass imguiPass{.name = "imguiPass"};
    imguiPass.m_ReadResources.push_back({"SwapChainImageCopy", ResourceUsage::TRANSFER_DESTINATION});
    imguiPass.m_ReadResources.push_back({"DrawImageGeometry", ResourceUsage::SHADER_READ});
    imguiPass.m_WriteResources.push_back({"SwapChainImage", ResourceUsage::COLOR_ATTACHMENT_WRITE});
    imguiPass.execute = [this](VkCommandBuffer cmd) {
        drawImgui(cmd, m_SwapChainImageViews[swapChainImageIndex]);
    };
    RenderPass copyPass{.name = "copyPass"};
    copyPass.m_ReadResources.push_back({"DrawImageGeometry", ResourceUsage::TRANSFER_SOURCE});
    copyPass.m_WriteResources.push_back({"SwapChainImageCopy", ResourceUsage::TRANSFER_DESTINATION});
    copyPass.execute = [this](VkCommandBuffer cmd) {
        if (!m_RenderToIMGUITexture) {
            VKImages::copyImageToImage(cmd, m_DrawImage.image, m_SwapChainImages[swapChainImageIndex],
                m_DrawExtent, m_SwapChainExtent);
        }
    };

    RenderPass presentPass{.name = "presentPass"};
    presentPass.m_ReadResources.push_back({"SwapChainImage", ResourceUsage::PRESENT_SRC});
    presentPass.execute = [this](VkCommandBuffer cmd) {
    };

    m_FrameGraph.registerDependency("DrawImageCompute", m_DrawImage.image);
    m_FrameGraph.registerDependency("DrawImageGeometry", m_DrawImage.image);
    m_FrameGraph.registerDependency("DepthImage", m_DepthImage.image);

    //No need for order since topologically sorted
    m_FrameGraph.addRenderPass(std::move(presentPass));
    m_FrameGraph.addRenderPass(std::move(geometryPass));
    m_FrameGraph.addRenderPass(std::move(imguiPass));
    m_FrameGraph.addRenderPass(std::move(computePass));
    m_FrameGraph.addRenderPass(std::move(copyPass));
}



void RenderContext::beginFrame() {
    VK_CHECK(vkWaitForFences(m_StateCtx->m_Device, 1, &getCurrentFrame().m_Fence, true, 1000000000));
    getCurrentFrame().m_DeletionQueue.flush();
    getCurrentFrame().m_FrameDescriptors.clearPools(m_StateCtx->m_Device);
    VK_CHECK(vkResetFences(m_StateCtx->m_Device, 1, &getCurrentFrame().m_Fence));

    auto result = vkAcquireNextImageKHR(m_StateCtx->m_Device, m_SwapChain, 1000000000,
        getCurrentFrame().m_SwapChainSemaphore,
        nullptr, &swapChainImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            //recreate swapchain
        m_SwapChainRecreated = true;
        return;
        }

    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    VkCommandBufferBeginInfo cmdBeginInfo = VKInit::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_DrawExtent.width = m_DrawImage.imageExtent.width;
    m_DrawExtent.height = m_DrawImage.imageExtent.height;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
}



void RenderContext::draw() {
    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;

    m_FrameGraph.registerDependency("SwapChainImageCopy", m_SwapChainImages[swapChainImageIndex]);
    m_FrameGraph.registerDependency("SwapChainImage", m_SwapChainImages[swapChainImageIndex]);

    m_FrameGraph.compileAndExecute(cmd);
}
void RenderContext::endFrame() {
    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdSubmitInfo = VKInit::cmdBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo waitInfo = VKInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        getCurrentFrame().m_SwapChainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = VKInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        getCurrentFrame().m_RenderSemaphore);

    VkSubmitInfo2 submit = VKInit::submitInfo(&cmdSubmitInfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(m_StateCtx->m_GraphicsQueue, 1, &submit, getCurrentFrame().m_Fence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &getCurrentFrame().m_RenderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapChainImageIndex;

    auto result = vkQueuePresentKHR(m_StateCtx->m_GraphicsQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        //recreate swapchain
        m_SwapChainRecreated = true;
    }

    m_FrameNumber++;

    if (m_SwapChainRecreated) {
        recreateSwapChain();
    }
}



void RenderContext::drawImgui(VkCommandBuffer cmd, VkImageView targetImageView) {
    VkRenderingAttachmentInfo colorAttachment = VKInit::attachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VKInit::renderingInfo(m_SwapChainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

std::vector<IndirectDrawCall> RenderContext::processDrawCalls() {
    std::sort(m_DrawCalls.begin(), m_DrawCalls.end(), [](const DrawCall &a, const DrawCall &b) {
        if (a.meshID != b.meshID)
            return a.meshID < b.meshID;

        return a.materialID< b.materialID;
    });

    auto* objectUniformData = (GPUObjectData*)getCurrentFrame().m_ObjectBuffer.allocationInfo.pMappedData;

    auto* indirectData = (VkDrawIndexedIndirectCommand*)getCurrentFrame().m_IndirectBuffer.allocationInfo.pMappedData;

    int i = 0;
    std::vector<IndirectDrawCall> indirectDrawCalls{};
    for (auto& call : m_DrawCalls) {
        if (!m_ResourceCtx->m_MeshMap.contains(call.meshID) || !m_ResourceCtx->m_MaterialMap.contains(call.materialID)) {
            spdlog::warn("Renderer: Trying to render invalid draw");
            continue;
        }
        auto& buf = m_ResourceCtx->m_MeshMap[call.meshID];
        auto& gpuMat = m_ResourceCtx->m_MaterialMap[call.materialID];


        if (!indirectDrawCalls.empty() && &buf == indirectDrawCalls.back().mesh &&
            &gpuMat == indirectDrawCalls.back().material) {
            indirectDrawCalls.back().count++;
            }
        else {
                IndirectDrawCall indirectDrawCall = {
                    .mesh = &buf,
                    .material = &gpuMat,
                    .drawIndex = i,
                    .count = 1,
                };

                indirectDrawCalls.push_back(indirectDrawCall);
        }

        objectUniformData[i] = {
            .matIndex = m_ResourceCtx->m_MaterialMap[call.materialID].idx,
            .model = call.model,
            .vertexBuffer = buf.vertexBufferAddress
        };

        indirectData[i].firstInstance = i;
        indirectData[i].firstIndex = 0;
        indirectData[i].instanceCount = 1;
        indirectData[i].indexCount = buf.indexCount;

        i++;
    }
    m_DrawCalls.clear();

    return indirectDrawCalls;
}



static void prepareViewport(VkCommandBuffer cmd, VkExtent2D extent) {
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = extent.width;
    viewport.height = extent.height;
    viewport.minDepth = 1.0f;
    viewport.maxDepth = 0.0f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = extent.width;
    scissor.extent.height = extent.height;

    vkCmdSetScissor(cmd, 0, 1, &scissor);
}



void RenderContext::drawGeometry(VkCommandBuffer cmd) {
    m_ResourceCtx->prepareDescriptors(cmd);

    const std::vector<IndirectDrawCall> indirectDrawCalls = processDrawCalls();

    //Split draws into two groups
    std::vector<IndirectDrawCall> depthDraws;
    std::vector<IndirectDrawCall> noDepthDraws;

    for (auto& draw : indirectDrawCalls) {
        if (draw.material->useDepth) {
            depthDraws.push_back(draw);
        } else {
            noDepthDraws.push_back(draw);
        }
    }

    if (!noDepthDraws.empty()) {
        VkRenderingAttachmentInfo colorAttInfo = VKInit::attachmentInfo(
            m_DrawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingInfo renderInfo = VKInit::renderingInfo(m_DrawExtent, &colorAttInfo, nullptr);

        vkCmdBeginRendering(cmd, &renderInfo);
        prepareViewport(cmd, m_DrawExtent);

        executeDrawCalls(cmd, noDepthDraws);
        vkCmdEndRendering(cmd);
    }
    if (!depthDraws.empty()) {
        VkRenderingAttachmentInfo colorAttInfo = VKInit::attachmentInfo(
            m_DrawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingAttachmentInfo depthAttInfo = VKInit::depthAttachmentInfo(
            m_DepthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
        VkRenderingInfo renderInfo = VKInit::renderingInfo(m_DrawExtent, &colorAttInfo, &depthAttInfo);

        vkCmdBeginRendering(cmd, &renderInfo);
        prepareViewport(cmd, m_DrawExtent);
        executeDrawCalls(cmd, depthDraws);
        vkCmdEndRendering(cmd);
    }
}

void RenderContext::executeDrawCalls(VkCommandBuffer cmd, const std::vector<IndirectDrawCall>& draws) {
    VkPipeline prevPipeline{};
    for (auto& draw : draws) {
        if (prevPipeline != draw.material->pipeline) {
            prevPipeline = draw.material->pipeline;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline);
        }
        vkCmdBindIndexBuffer(cmd, draw.mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexedIndirect(cmd, getCurrentFrame().m_IndirectBuffer.buffer,
            sizeof(VkDrawIndexedIndirectCommand) * draw.drawIndex, draw.count,
            sizeof(VkDrawIndexedIndirectCommand));
    }
}

void RenderContext::cleanup() {
    vkDeviceWaitIdle(m_StateCtx->m_Device);
    for (int i{}; i < FRAME_OVERLAP; ++i) {
        vkDestroyCommandPool(m_StateCtx->m_Device, m_Frames[i].m_CommandPool, nullptr);

        vkDestroyFence(m_StateCtx->m_Device, m_Frames[i].m_Fence, nullptr);
        vkDestroySemaphore(m_StateCtx->m_Device, m_Frames[i].m_RenderSemaphore, nullptr);
        vkDestroySemaphore(m_StateCtx->m_Device, m_Frames[i].m_SwapChainSemaphore, nullptr);
        m_Frames[i].m_DeletionQueue.flush();
    }
    destroySwapChain();
}

void RenderContext::immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function) {
    VK_CHECK(vkResetFences(m_StateCtx->m_Device, 1, &m_ImmediateFence));
    VK_CHECK(vkResetCommandPool(m_StateCtx->m_Device, m_ImmediateCommandPool, 0));

    VkCommandBuffer cmd = m_ImmediateCommandBuffer;

    VkCommandBufferBeginInfo beginInfo = VKInit::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdInfo = VKInit::cmdBufferSubmitInfo(cmd);
    VkSubmitInfo2 submitInfo = VKInit::submitInfo(&cmdInfo, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(m_StateCtx->m_GraphicsQueue, 1, &submitInfo, m_ImmediateFence));
    VK_CHECK(vkWaitForFences(m_StateCtx->m_Device, 1, &m_ImmediateFence, VK_TRUE, UINT64_MAX));
}
void RenderContext::initSwapChain() {
    int width, height;
    glfwGetFramebufferSize(Application::get().getWindow()->getNativeWindow(), &width, &height);
    createSwapChain(width, height);

    VkExtent3D drawImageExtent {
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .depth = 1
    };

    m_DrawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_DrawImage.imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo rImgInfo = VKInit::imageCreateInfo(m_DrawImage.imageFormat, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo rImgAllocationCreateInfo {};
    rImgAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rImgAllocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(m_ResourceCtx->m_Allocator, &rImgInfo, &rImgAllocationCreateInfo, &m_DrawImage.image, &m_DrawImage.allocation, nullptr);

    VkImageViewCreateInfo rViewInfo = VKInit::imageViewCreateInfo(m_DrawImage.image, m_DrawImage.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(m_StateCtx->m_Device, &rViewInfo, nullptr, &m_DrawImage.imageView));

    m_DepthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
    m_DepthImage.imageExtent = drawImageExtent;
    VkImageUsageFlags depthImageUsages{};
    depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkImageCreateInfo depthImageInfo = VKInit::imageCreateInfo(m_DepthImage.imageFormat, depthImageUsages, drawImageExtent);
    vmaCreateImage(m_ResourceCtx->m_Allocator, &depthImageInfo, &rImgAllocationCreateInfo, &m_DepthImage.image,
        &m_DepthImage.allocation, nullptr);

    VkImageViewCreateInfo depthViewInfo = VKInit::imageViewCreateInfo(m_DepthImage.image,
        m_DepthImage.imageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    VK_CHECK(vkCreateImageView(m_StateCtx->m_Device, &depthViewInfo, nullptr, &m_DepthImage.imageView));

    m_ResourceCtx->m_DeletionQueue.pushFunction([=]() {
        vkDestroyImageView(m_StateCtx->m_Device, m_DrawImage.imageView, nullptr);
        vmaDestroyImage(m_ResourceCtx->m_Allocator, m_DrawImage.image, m_DrawImage.allocation);

        vkDestroyImageView(m_StateCtx->m_Device, m_DepthImage.imageView, nullptr);
        vmaDestroyImage(m_ResourceCtx->m_Allocator, m_DepthImage.image, m_DepthImage.allocation);
    });
    spdlog::debug("Renderer: Initialized Swapchain");
}
void RenderContext::createSwapChain(uint32_t width, uint32_t height) {
    vkb::SwapchainBuilder swapChainBuilder{m_StateCtx->m_PhysicalDevice, m_StateCtx->m_Device, m_StateCtx->m_Surface};

    m_SwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    swapChainBuilder.set_desired_format(VkSurfaceFormatKHR{.format = m_SwapChainImageFormat});
    //swapChainBuilder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    swapChainBuilder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
    swapChainBuilder.set_desired_extent(width, height);
    swapChainBuilder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    swapChainBuilder.set_desired_min_image_count(FRAME_OVERLAP);
    vkb::Swapchain vkbSwapChain = swapChainBuilder.build().value();

    m_SwapChainExtent = vkbSwapChain.extent;

    m_SwapChain = vkbSwapChain.swapchain;
    m_SwapChainImages = vkbSwapChain.get_images().value();
    m_SwapChainImageViews = vkbSwapChain.get_image_views().value();

    spdlog::debug("Renderer: Swapchain Created");

}

void RenderContext::recreateSwapChain() {
    int width, height;
    glfwGetFramebufferSize(Application::get().getWindow()->getNativeWindow(), &width, &height);
    vkDeviceWaitIdle(m_StateCtx->m_Device);
    destroySwapChain();
    m_DrawExtent.width = width;
    m_DrawExtent.height = height;
    createSwapChain(width, height);
    m_SwapChainRecreated = false;
    spdlog::debug("Renderer: Swapchain Recreated");
}

void RenderContext::destroySwapChain() {
    vkDestroySwapchainKHR(m_StateCtx->m_Device, m_SwapChain, nullptr);

    for (size_t i{}; i < m_SwapChainImageViews.size(); ++i) {
        vkDestroyImageView(m_StateCtx->m_Device,m_SwapChainImageViews[i], nullptr);
    }
    spdlog::debug("Renderer: Swapchain Destroyed");
}
void RenderContext::initSyncStructures() {
    VkFenceCreateInfo fenceCreateInfo = VKInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = VKInit::semaphoreCreateInfo();

    for (int i{}; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateFence(m_StateCtx->m_Device, &fenceCreateInfo, nullptr, &m_Frames[i].m_Fence));
        VK_CHECK(vkCreateSemaphore(m_StateCtx->m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].m_SwapChainSemaphore));
        VK_CHECK(vkCreateSemaphore(m_StateCtx->m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].m_RenderSemaphore));
    }
    VK_CHECK(vkCreateFence(m_StateCtx->m_Device, &fenceCreateInfo, nullptr, &m_ImmediateFence));
    m_ResourceCtx->m_DeletionQueue.pushFunction([=]() {
        vkDestroyFence(m_StateCtx->m_Device, m_ImmediateFence, nullptr);
    });
    spdlog::debug("Renderer: Initialized Sync Structures");
}
