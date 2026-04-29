#include "ZeusEngineCore/engine/rendering/VKRenderer.h"

#include "ZeusEngineCore/core/Application.h"
#include <vulkan/vk_enum_string_helper.h>
#include "VKImages.h"
#include "VKInit.h"
#include "VKPipelines.h"
#include "ZeusEngineCore/engine/CameraSystem.h"
#include "ZeusEngineCore/engine/Scene.h"
#include "ZeusEngineCore/engine/rendering/VKUtils.h"
#include "ZeusEngineCore/engine/Components.h"
#include <vma/vk_mem_alloc.h>

using namespace ZEN;

uint32_t IndexAllocator::allocate() {
    if (!availableList.empty()) {
        uint32_t idx = availableList.back();
        availableList.pop_back();
        return idx;
    }
    spdlog::warn("Renderer: No available slot for texture!");
    return 0;
}

void IndexAllocator::free(uint32_t idx) {
    freeList.push_back(idx);
}

void IndexAllocator::init(uint32_t max) {
    availableList.resize(max);
    for (uint32_t i = 0; i < max; i++) {
        availableList[i] = max - 1 - i;
    }
}

void IndexAllocator::flush() {
    availableList.insert(availableList.end(), freeList.begin(), freeList.end());
    freeList.clear();
}



static uint32_t swapChainImageIndex{};
void VKRenderer::beginFrame() {
    VK_CHECK(vkWaitForFences(m_Device, 1, &getCurrentFrame().m_Fence, true, 1000000000));
    getCurrentFrame().m_DeletionQueue.flush();
    getCurrentFrame().m_FrameDescriptors.clearPools(m_Device);
    VK_CHECK(vkResetFences(m_Device, 1, &getCurrentFrame().m_Fence));

    auto result = vkAcquireNextImageKHR(m_Device, m_SwapChain, 1000000000, getCurrentFrame().m_SwapChainSemaphore,
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

    //make SwapChain writable
    VKImages::transitionImage(cmd, m_DrawImage.image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}



void VKRenderer::draw() {
    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;

    drawBackground(cmd);
    VKImages::transitionImage(cmd, m_DrawImage.image,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
     VKImages::transitionImage(cmd, m_DepthImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    drawGeometry(cmd);

}
void VKRenderer::endFrame() {
    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;

    if (!m_RenderToIMGUITexture) {
        VKImages::transitionImage(cmd, m_DrawImage.image,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        VKImages::copyImageToImage(cmd, m_DrawImage.image, m_SwapChainImages[swapChainImageIndex],
            m_DrawExtent, m_SwapChainExtent);

        //we still draw imgui stuff, but we dont render to an imgui texture
        VKImages::transitionImage(cmd, m_DrawImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
        VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        drawImgui(cmd, m_SwapChainImageViews[swapChainImageIndex]);
        VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    } else {
        VKImages::transitionImage(cmd, m_DrawImage.image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        drawImgui(cmd, m_SwapChainImageViews[swapChainImageIndex]);
        VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        /*--------------------------------------------------------IMGUI------------------------------------------------*/
    }
    //make swapchain presentable
    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdSubmitInfo = VKInit::cmdBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo waitInfo = VKInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        getCurrentFrame().m_SwapChainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = VKInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        getCurrentFrame().m_RenderSemaphore);

    VkSubmitInfo2 submit = VKInit::submitInfo(&cmdSubmitInfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submit, getCurrentFrame().m_Fence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &getCurrentFrame().m_RenderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapChainImageIndex;

    auto result = vkQueuePresentKHR(m_GraphicsQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        //recreate swapchain
        m_SwapChainRecreated = true;
    }

    m_FrameNumber++;

    if (m_SwapChainRecreated) {
        recreateSwapChain();
    }
}

void VKRenderer::submitDrawCall(const DrawCall &call) {
    m_DrawCalls.push_back(call);
}

void VKRenderer::drawBackground(VkCommandBuffer cmd) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientPipelineLayout,
        0, 1, &m_DrawImageDescriptors, 0, nullptr);
    float dt = Application::get().getWindow()->getDeltaTime();
    static float totalTime = 0;
    totalTime += dt;
    vkCmdPushConstants(cmd, m_GradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float), &totalTime);

    vkCmdDispatch(cmd, std::ceil(m_DrawExtent.width / 16.0),
        std::ceil(m_DrawExtent.height / 16.0), 1);
}

void VKRenderer::drawImgui(VkCommandBuffer cmd, VkImageView targetImageView) {
    VkRenderingAttachmentInfo colorAttachment = VKInit::attachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VKInit::renderingInfo(m_SwapChainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

std::vector<IndirectDrawCall> VKRenderer::processDrawCalls() {
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
        auto& buf = m_MeshMap[call.meshID];
        auto& gpuMat = m_MaterialMap[call.materialID];

        if (!indirectDrawCalls.empty() && &buf == indirectDrawCalls.back().mesh &&
            &gpuMat.first == indirectDrawCalls.back().material) {
            indirectDrawCalls.back().count++;
            } else {
                IndirectDrawCall indirectDrawCall = {
                    .mesh = &buf,
                    .material = &gpuMat.first,
                    .drawIndex = i,
                    .count = 1,
                };

                indirectDrawCalls.push_back(indirectDrawCall);
            }

        objectUniformData[i] = {
            .matIndex = m_MaterialMap[call.materialID].second,
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

void VKRenderer::prepareDescriptors(VkCommandBuffer cmd) {
    //write to scene data buffer
    auto lightDir = m_Scene->getLightDir();
    glm::vec4 light = glm::vec4(lightDir.x, lightDir.y, lightDir.z, 1);
    auto cameraPos = m_Scene->getSceneCamera().getComponent<TransformComp>().getWorldPosition();

    auto* sceneUniformData = (GPUSceneData*)getCurrentFrame().m_SceneBuffer.allocationInfo.pMappedData;
    m_SceneData = {};
    m_SceneData.viewProj = m_CameraSystem->getVP();
    m_SceneData.ambientColor = {0.5f, 0.5f, 0.5f, 1.0f};
    m_SceneData.sunlightColor = {1.0f, 1.0f, 1.0f, 1.0f};
    m_SceneData.sunlightDirection = light;
    m_SceneData.cameraPosition = glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1);

    *sceneUniformData = m_SceneData;

    VkDescriptorSet globalDescriptor = getCurrentFrame().m_FrameDescriptors.
    allocate(m_Device, m_FrameDescriptorLayout);

    DescriptorWriter writer;
    writer.writeBuffer(0, getCurrentFrame().m_SceneBuffer.buffer,
        sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    writer.writeBuffer(1, getCurrentFrame().m_ObjectBuffer.buffer,
        sizeof(GPUObjectData) * 1000, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    writer.updateSet(m_Device, globalDescriptor);

    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
    m_MeshPipelineLayout,0, 1, &globalDescriptor,0,nullptr);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_MeshPipelineLayout,1, 1, &m_TextureDescriptorSet,0,nullptr);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_MeshPipelineLayout,2, 1, &m_MaterialDescriptorSet,0,nullptr);

}

void VKRenderer::drawGeometry(VkCommandBuffer cmd) {
    prepareDescriptors(cmd);

    const std::vector<IndirectDrawCall> indirectDrawCalls = processDrawCalls();

    VkRenderingAttachmentInfo colorAttInfo = VKInit::attachmentInfo(m_DrawImage.imageView
        , nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttInfo = VKInit::depthAttachmentInfo(m_DepthImage.imageView
        , VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VKInit::renderingInfo(m_DrawExtent,
        &colorAttInfo, &depthAttInfo);

    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshPipeline);

    prepareViewport(cmd, m_DrawExtent);

    for (auto& draw : indirectDrawCalls) {
        vkCmdBindIndexBuffer(cmd, draw.mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexedIndirect(cmd, getCurrentFrame().m_IndirectBuffer.buffer,
            sizeof(VkDrawIndexedIndirectCommand) * draw.drawIndex, draw.count,
            sizeof(VkDrawIndexedIndirectCommand));
    }
    vkCmdEndRendering(cmd);
}

void VKRenderer::immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function) {
    VK_CHECK(vkResetFences(m_Device, 1, &m_ImmediateFence));
    VK_CHECK(vkResetCommandPool(m_Device, m_ImmediateCommandPool, 0));

    VkCommandBuffer cmd = m_ImmediateCommandBuffer;

    VkCommandBufferBeginInfo beginInfo = VKInit::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdInfo = VKInit::cmdBufferSubmitInfo(cmd);
    VkSubmitInfo2 submitInfo = VKInit::submitInfo(&cmdInfo, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submitInfo, m_ImmediateFence));
    VK_CHECK(vkWaitForFences(m_Device, 1, &m_ImmediateFence, VK_TRUE, UINT64_MAX));
}

ImGui_ImplVulkan_InitInfo VKRenderer::initImgui() {
    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &imguiPool));

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_Instance;
    initInfo.PhysicalDevice = m_PhysicalDevice;
    initInfo.Device = m_Device;
    initInfo.Queue = m_GraphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;

    //dynamic rendering parameters stuff
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_SwapChainImageFormat;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
    m_ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_DrawImage.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    m_ImGUIErrorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ErrorTexture.image.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );


    m_DeletionQueue.pushFunction([=]() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(m_Device, imguiPool, nullptr);

    });
    spdlog::debug("Renderer: ImGUI Initialized");

    return initInfo;
}

VkDescriptorSet VKRenderer::getImGUIDescSet(AssetID id) {
    if (m_ImGUIDescSetMap.contains(id)) {
        return m_ImGUIDescSetMap[id];
    }
    //if not found, add this id for cache
    if (m_TextureMap.contains(id)) {
        spdlog::debug("Renderer: Cached Thumbnail Tex: {}", (uint64_t)id);
        auto& tex = m_TextureMap[id];
        m_ImGUIDescSetMap.insert({id, ImGui_ImplVulkan_AddTexture(m_Sampler, tex.first.image.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)});
        return m_ImGUIDescSetMap[id];
    }
    return m_ImGUIErrorSet;
}

