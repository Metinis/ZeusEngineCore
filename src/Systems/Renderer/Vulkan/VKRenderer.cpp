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

#include "SkyboxRenderer.h"
#include "VkHelpers.h"

using namespace ZEN;

uint32_t IndexAllocator::allocate() {
    if (!availableList.empty()) {
        uint32_t idx = availableList.back();
        availableList.pop_back();
        return idx;
    }
    spdlog::error("Renderer: No available slot for texture!");
    return 0;
}

uint32_t IndexAllocator::allocate(uint32_t idx) {
    auto it = std::find(availableList.begin(), availableList.end(), idx);
    if (it != availableList.end()) {
        availableList.erase(it);
        return idx;
    }

    spdlog::error("Renderer: Requested index {} not available!", idx);
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

void VKRenderer::initFrameGraph() {
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
        drawImgui(cmd, m_RenderCtx.m_SwapchainCtx.m_SwapChainImageViews[swapChainImageIndex]);
    };
    RenderPass copyPass{.name = "copyPass"};
    copyPass.m_ReadResources.push_back({"DrawImageGeometry", ResourceUsage::TRANSFER_SOURCE});
    copyPass.m_WriteResources.push_back({"SwapChainImageCopy", ResourceUsage::TRANSFER_DESTINATION});
    copyPass.execute = [this](VkCommandBuffer cmd) {
        if (!m_RenderCtx.m_RenderToIMGUITexture) {
            VKImages::copyImageToImage(cmd, m_RenderCtx.m_DrawImage.image, m_RenderCtx.m_SwapchainCtx.m_SwapChainImages[swapChainImageIndex],
                m_RenderCtx.m_DrawExtent, m_RenderCtx.m_SwapchainCtx.m_SwapChainExtent);
        }
    };

    RenderPass presentPass{.name = "presentPass"};
    presentPass.m_ReadResources.push_back({"SwapChainImage", ResourceUsage::PRESENT_SRC});
    presentPass.execute = [this](VkCommandBuffer cmd) {
    };

    m_RenderCtx.m_FrameGraph.registerDependency("DrawImageCompute", m_RenderCtx.m_DrawImage.image);
    m_RenderCtx.m_FrameGraph.registerDependency("DrawImageGeometry", m_RenderCtx.m_DrawImage.image);
    m_RenderCtx.m_FrameGraph.registerDependency("DepthImage", m_RenderCtx.m_DepthImage.image);

    //No need for order since topologically sorted
    m_RenderCtx.m_FrameGraph.addRenderPass(std::move(presentPass));
    m_RenderCtx.m_FrameGraph.addRenderPass(std::move(geometryPass));
    m_RenderCtx.m_FrameGraph.addRenderPass(std::move(imguiPass));
    m_RenderCtx.m_FrameGraph.addRenderPass(std::move(computePass));
    m_RenderCtx.m_FrameGraph.addRenderPass(std::move(copyPass));
}


void VKRenderer::beginFrame() {
    VK_CHECK(vkWaitForFences(m_StateCtx.m_Device, 1, &m_RenderCtx.getCurrentFrame().m_Fence, true, 1000000000));
    m_RenderCtx.getCurrentFrame().m_DeletionQueue.flush();
    m_RenderCtx.getCurrentFrame().m_FrameDescriptors.clearPools(m_StateCtx.m_Device);
    VK_CHECK(vkResetFences(m_StateCtx.m_Device, 1, &m_RenderCtx.getCurrentFrame().m_Fence));

    auto result = vkAcquireNextImageKHR(m_StateCtx.m_Device, m_RenderCtx.m_SwapchainCtx.m_SwapChain, 1000000000,
        m_RenderCtx.getCurrentFrame().m_SwapChainSemaphore,
        nullptr, &swapChainImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            //recreate swapchain
        m_RenderCtx.m_SwapchainCtx.m_SwapChainRecreated = true;
        return;
        }

    VkCommandBuffer cmd = m_RenderCtx.getCurrentFrame().m_MainCommandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    VkCommandBufferBeginInfo cmdBeginInfo = VKInit::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_RenderCtx.m_DrawExtent.width = m_RenderCtx.m_DrawImage.imageExtent.width;
    m_RenderCtx.m_DrawExtent.height = m_RenderCtx.m_DrawImage.imageExtent.height;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
}



void VKRenderer::draw() {
    VkCommandBuffer cmd = m_RenderCtx.getCurrentFrame().m_MainCommandBuffer;

    m_RenderCtx.m_FrameGraph.registerDependency("SwapChainImageCopy", m_RenderCtx.m_SwapchainCtx.m_SwapChainImages[swapChainImageIndex]);
    m_RenderCtx.m_FrameGraph.registerDependency("SwapChainImage", m_RenderCtx.m_SwapchainCtx.m_SwapChainImages[swapChainImageIndex]);

    m_RenderCtx.m_FrameGraph.compileAndExecute(cmd);
}
void VKRenderer::endFrame() {
    VkCommandBuffer cmd = m_RenderCtx.getCurrentFrame().m_MainCommandBuffer;

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdSubmitInfo = VKInit::cmdBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo waitInfo = VKInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        m_RenderCtx.getCurrentFrame().m_SwapChainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = VKInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        m_RenderCtx.getCurrentFrame().m_RenderSemaphore);

    VkSubmitInfo2 submit = VKInit::submitInfo(&cmdSubmitInfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(m_StateCtx.m_GraphicsQueue, 1, &submit, m_RenderCtx.getCurrentFrame().m_Fence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_RenderCtx.m_SwapchainCtx.m_SwapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &m_RenderCtx.getCurrentFrame().m_RenderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapChainImageIndex;

    auto result = vkQueuePresentKHR(m_StateCtx.m_GraphicsQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        //recreate swapchain
        m_RenderCtx.m_SwapchainCtx.m_SwapChainRecreated = true;
    }

    m_RenderCtx.m_FrameNumber++;

    if (m_RenderCtx.m_SwapchainCtx.m_SwapChainRecreated) {
        recreateSwapChain();
    }
}

void VKRenderer::submitDrawCall(const DrawCall &call) {
    m_RenderCtx.m_DrawCalls.push_back(call);
}

void VKRenderer::setImGUIMode(const bool mode) {
    m_RenderCtx.m_RenderToIMGUITexture = mode;
}

void VKRenderer::drawImgui(VkCommandBuffer cmd, VkImageView targetImageView) {
    VkRenderingAttachmentInfo colorAttachment = VKInit::attachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VKInit::renderingInfo(m_RenderCtx.m_SwapchainCtx.m_SwapChainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

std::vector<IndirectDrawCall> VKRenderer::processDrawCalls() {
    std::sort(m_RenderCtx.m_DrawCalls.begin(), m_RenderCtx.m_DrawCalls.end(), [](const DrawCall &a, const DrawCall &b) {
        if (a.meshID != b.meshID)
            return a.meshID < b.meshID;

        return a.materialID< b.materialID;
    });

    auto* objectUniformData = (GPUObjectData*)m_RenderCtx.getCurrentFrame().m_ObjectBuffer.allocationInfo.pMappedData;

    auto* indirectData = (VkDrawIndexedIndirectCommand*)m_RenderCtx.getCurrentFrame().m_IndirectBuffer.allocationInfo.pMappedData;

    int i = 0;
    std::vector<IndirectDrawCall> indirectDrawCalls{};
    for (auto& call : m_RenderCtx.m_DrawCalls) {
        if (!m_ResourceCtx.m_MeshMap.contains(call.meshID) || !m_ResourceCtx.m_MaterialMap.contains(call.materialID)) {
            spdlog::warn("Renderer: Trying to render invalid draw");
            continue;
        }
        auto& buf = m_ResourceCtx.m_MeshMap[call.meshID];
        auto& gpuMat = m_ResourceCtx.m_MaterialMap[call.materialID];


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
            .matIndex = m_ResourceCtx.m_MaterialMap[call.materialID].idx,
            .model = call.model,
            .vertexBuffer = buf.vertexBufferAddress
        };

        indirectData[i].firstInstance = i;
        indirectData[i].firstIndex = 0;
        indirectData[i].instanceCount = 1;
        indirectData[i].indexCount = buf.indexCount;

        i++;
    }
    m_RenderCtx.m_DrawCalls.clear();

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
    auto cameraPos = m_Scene->getCamera().getComponent<TransformComp>().getWorldPosition();

    auto* sceneUniformData = (GPUSceneData*)m_RenderCtx.getCurrentFrame().m_SceneBuffer.allocationInfo.pMappedData;
    m_ResourceCtx.m_SceneData = {};
    m_ResourceCtx.m_SceneData.view = m_CameraSystem->getView();
    m_ResourceCtx.m_SceneData.proj = m_CameraSystem->getProjection();
    m_ResourceCtx.m_SceneData.ambientColor = {0.5f, 0.5f, 0.5f, 1.0f};
    m_ResourceCtx.m_SceneData.sunlightColor = {1.0f, 1.0f, 1.0f, 1.0f};
    m_ResourceCtx.m_SceneData.sunlightDirection = light;
    m_ResourceCtx.m_SceneData.cameraPosition = glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1);

    *sceneUniformData = m_ResourceCtx.m_SceneData;

    VkDescriptorSet globalDescriptor = m_RenderCtx.getCurrentFrame().m_FrameDescriptors.
    allocate(m_StateCtx.m_Device, m_ResourceCtx.m_FrameDescriptorLayout);

    DescriptorWriter writer;
    writer.writeBuffer(0, m_RenderCtx.getCurrentFrame().m_SceneBuffer.buffer,
        sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    writer.writeBuffer(1, m_RenderCtx.getCurrentFrame().m_ObjectBuffer.buffer,
        sizeof(GPUObjectData) * 10000, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    writer.updateSet(m_StateCtx.m_Device, globalDescriptor);

    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
    m_ResourceCtx.m_MainPipelineLayout,0, 1, &globalDescriptor,0,nullptr);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_ResourceCtx.m_MainPipelineLayout,1, 1, &m_ResourceCtx.m_TextureDescriptorSet,0,nullptr);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_ResourceCtx.m_MainPipelineLayout,2, 1, &m_ResourceCtx.m_MaterialDescriptorSet,0,nullptr);

    GPUMainPushConstants pc {
        .skyboxIdx = m_SkyboxRenderer->getSkyboxReadIdx(),
        .irradianceIdx = m_SkyboxRenderer->getIrradianceReadIdx(),
        .prefilterMapIdx = m_SkyboxRenderer->getPrefilterReadIdx(),
        .brdfTexIdx = m_SkyboxRenderer->getBRDFReadIdx(),
    };
    vkCmdPushConstants(cmd, m_ResourceCtx.m_MainPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUMainPushConstants), &pc);

}

void VKRenderer::drawGeometry(VkCommandBuffer cmd) {
    prepareDescriptors(cmd);

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
            m_RenderCtx.m_DrawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingInfo renderInfo = VKInit::renderingInfo(m_RenderCtx.m_DrawExtent, &colorAttInfo, nullptr);

        vkCmdBeginRendering(cmd, &renderInfo);
        prepareViewport(cmd, m_RenderCtx.m_DrawExtent);

        executeDrawCalls(cmd, noDepthDraws);
        vkCmdEndRendering(cmd);
    }
    if (!depthDraws.empty()) {
        VkRenderingAttachmentInfo colorAttInfo = VKInit::attachmentInfo(
            m_RenderCtx.m_DrawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingAttachmentInfo depthAttInfo = VKInit::depthAttachmentInfo(
            m_RenderCtx.m_DepthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
        VkRenderingInfo renderInfo = VKInit::renderingInfo(m_RenderCtx.m_DrawExtent, &colorAttInfo, &depthAttInfo);

        vkCmdBeginRendering(cmd, &renderInfo);
        prepareViewport(cmd, m_RenderCtx.m_DrawExtent);
        executeDrawCalls(cmd, depthDraws);
        vkCmdEndRendering(cmd);
    }
}

void VKRenderer::executeDrawCalls(VkCommandBuffer cmd, const std::vector<IndirectDrawCall>& draws) {
    VkPipeline prevPipeline{};
    for (auto& draw : draws) {
        if (prevPipeline != draw.material->pipeline) {
            prevPipeline = draw.material->pipeline;
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline);
        }
        vkCmdBindIndexBuffer(cmd, draw.mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexedIndirect(cmd, m_RenderCtx.getCurrentFrame().m_IndirectBuffer.buffer,
            sizeof(VkDrawIndexedIndirectCommand) * draw.drawIndex, draw.count,
            sizeof(VkDrawIndexedIndirectCommand));
    }
}

void VKRenderer::immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function) {
    VK_CHECK(vkResetFences(m_StateCtx.m_Device, 1, &m_RenderCtx.m_ImmediateFence));
    VK_CHECK(vkResetCommandPool(m_StateCtx.m_Device, m_RenderCtx.m_ImmediateCommandPool, 0));

    VkCommandBuffer cmd = m_RenderCtx.m_ImmediateCommandBuffer;

    VkCommandBufferBeginInfo beginInfo = VKInit::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdInfo = VKInit::cmdBufferSubmitInfo(cmd);
    VkSubmitInfo2 submitInfo = VKInit::submitInfo(&cmdInfo, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(m_StateCtx.m_GraphicsQueue, 1, &submitInfo, m_RenderCtx.m_ImmediateFence));
    VK_CHECK(vkWaitForFences(m_StateCtx.m_Device, 1, &m_RenderCtx.m_ImmediateFence, VK_TRUE, UINT64_MAX));
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
    VK_CHECK(vkCreateDescriptorPool(m_StateCtx.m_Device, &poolInfo, nullptr, &imguiPool));

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_StateCtx.m_Instance;
    initInfo.PhysicalDevice = m_StateCtx.m_PhysicalDevice;
    initInfo.Device = m_StateCtx.m_Device;
    initInfo.Queue = m_StateCtx.m_GraphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;

    //dynamic rendering parameters stuff
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_RenderCtx.m_SwapchainCtx.m_SwapChainImageFormat;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
    m_ResourceCtx.m_ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(getSampler(VKHelpers::getDefaultSamplerInfo()).sampler, m_RenderCtx.m_DrawImage.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    m_ResourceCtx.m_ImGUIErrorSet = ImGui_ImplVulkan_AddTexture(getSampler(VKHelpers::getDefaultSamplerInfo()).sampler, m_ResourceCtx.m_ErrorTexture.image.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );


    m_ResourceCtx.m_DeletionQueue.pushFunction([=]() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(m_StateCtx.m_Device, imguiPool, nullptr);

    });
    spdlog::debug("Renderer: ImGUI Initialized");

    return initInfo;
}

VkDescriptorSet VKRenderer::getImGUIDescSet(AssetID id) {
    if (m_ResourceCtx.m_ImGUIDescSetMap.contains(id)) {
        return m_ResourceCtx.m_ImGUIDescSetMap[id];
    }
    //if not found, add this id for cache
    //todo check the sketchy NULL HANDLE
    if (m_ResourceCtx.m_TextureMap.contains(id) && m_ResourceCtx.m_TextureMap[id].texture.image.imageView != VK_NULL_HANDLE) {
        spdlog::debug("Renderer: Cached Thumbnail Tex: {}", (uint64_t)id);
        auto& tex = m_ResourceCtx.m_TextureMap[id];
        if (tex.type == TextureType::Texture2D || tex.type == Texture2DAssimp) {
            m_ResourceCtx.m_ImGUIDescSetMap.insert({id, ImGui_ImplVulkan_AddTexture(getSampler(
                VKHelpers::getDefaultSamplerInfo()).sampler, tex.texture.image.imageView,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)});
        } else {
            m_ResourceCtx.m_ImGUIDescSetMap.insert({id, ImGui_ImplVulkan_AddTexture(getSampler(
                VKHelpers::getDefaultSamplerInfo()).sampler, m_ResourceCtx.m_ErrorTexture.image.imageView,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)});
        }

        return m_ResourceCtx.m_ImGUIDescSetMap[id];
    }
    return m_ResourceCtx.m_ImGUIErrorSet;
}

