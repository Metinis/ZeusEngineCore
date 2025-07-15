#include "APIRenderer.h"

using namespace ZEN::VKAPI;

APIRenderer::APIRenderer(APIBackend* apiBackend) : m_Backend(apiBackend){

}
void APIRenderer::BeginFrame()
{
    m_Backend->FlushDeferredDestroys();
    RenderSync& renderSync = m_FrameInfo.sync->GetRenderSyncAtFrame();
    vk::CommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    renderSync.commandBuffer.begin(commandBufferBeginInfo);
    m_CommandBuffer = renderSync.commandBuffer;
}
bool APIRenderer::AcquireRenderTarget()
{
    m_FrameInfo = m_Backend->GetRenderFrameInfo();
    //minimized
    if (m_FrameInfo.framebufferSize.x <= 0 || m_FrameInfo.framebufferSize.y <= 0) {
        return false;
    }

    RenderSync& renderSync = m_FrameInfo.sync->GetRenderSyncAtFrame();

    static constexpr std::uint64_t fenceTimeout_v = 3'000'000'000ull; // 3 seconds in nanoseconds
    vk::Result result = m_FrameInfo.device->GetLogicalDevice().waitForFences(*renderSync.drawn, vk::True, fenceTimeout_v);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error{ "Failed to wait for Render Fence" };
    }
    m_RenderTarget = m_FrameInfo.swapchain->AquireNextImage(*renderSync.draw);
    if (!m_RenderTarget) {
        // acquire failure
        m_FrameInfo.swapchain->Recreate(m_FrameInfo.framebufferSize);
        return false;
    }

    m_FrameInfo.device->GetLogicalDevice().resetFences(*renderSync.drawn);

    return true;
}


void APIRenderer::TransitionForRender() const
{
    auto barrier = m_FrameInfo.swapchain->GetBaseBarrier();
    auto dependency_info = vk::DependencyInfo{};
    barrier.setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eAttachmentOptimal)
            .setSrcAccessMask({})
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
            .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
            .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

    dependency_info.setImageMemoryBarriers(barrier);

    m_CommandBuffer.pipelineBarrier2(dependency_info);
}

void APIRenderer::Render(const std::function<void(vk::CommandBuffer, vk::Extent2D)>& drawCallback,
                        const std::function<void(vk::CommandBuffer)>& uiDrawCallback)
{
    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = m_RenderTarget->imageView;
    colorAttachment.imageLayout = vk::ImageLayout::eAttachmentOptimal;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f };

    vk::RenderingInfo renderingInfo{};
    auto const renderArea = vk::Rect2D{ vk::Offset2D{}, m_RenderTarget->extent };

    renderingInfo.renderArea = renderArea;
    renderingInfo.setColorAttachments(colorAttachment);
    renderingInfo.layerCount = 1;

    m_CommandBuffer.beginRendering(renderingInfo);
    //draw mesh stuff here
    if(drawCallback) drawCallback(m_CommandBuffer, m_RenderTarget->extent);
    m_CommandBuffer.endRendering();

    // UI pass
    colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
    renderingInfo.setColorAttachments(colorAttachment);
    renderingInfo.setPDepthAttachment(nullptr); // no depth

    m_CommandBuffer.beginRendering(renderingInfo);
    if(uiDrawCallback) uiDrawCallback(m_CommandBuffer);
    m_CommandBuffer.endRendering();
}

void APIRenderer::TransitionForPresent() const
{
    vk::DependencyInfo dependencyInfo{};
    auto barrier = m_FrameInfo.swapchain->GetBaseBarrier();

    barrier.oldLayout = vk::ImageLayout::eAttachmentOptimal;
    barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    barrier.dstAccessMask = barrier.srcAccessMask;
    barrier.dstStageMask = barrier.srcStageMask;

    dependencyInfo.setImageMemoryBarriers(barrier);
    m_CommandBuffer.pipelineBarrier2(dependencyInfo);
}

void APIRenderer::SubmitAndPresent()
{
    RenderSync& renderSync = m_FrameInfo.sync->GetRenderSyncAtFrame();
    renderSync.commandBuffer.end();

    vk::SubmitInfo2 submitInfo{};

    vk::CommandBufferSubmitInfo cmdBuffSubmitInfo{ renderSync.commandBuffer };

    vk::SemaphoreSubmitInfo waitSemaphoreInfo{};
    waitSemaphoreInfo.semaphore = *renderSync.draw;
    waitSemaphoreInfo.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

    vk::SemaphoreSubmitInfo signalSemaphoreInfo{};
    signalSemaphoreInfo.semaphore = m_FrameInfo.swapchain->GetPresentSemaphore();
    signalSemaphoreInfo.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

    std::array<vk::CommandBufferSubmitInfo, 1> cmdBuffInfos{ cmdBuffSubmitInfo };
    std::array<vk::SemaphoreSubmitInfo, 1> waitSemaphoreInfos{ waitSemaphoreInfo };
    std::array<vk::SemaphoreSubmitInfo, 1> signalSemaphoreInfos{ signalSemaphoreInfo };

    submitInfo.setCommandBufferInfos(cmdBuffInfos);
    submitInfo.setWaitSemaphoreInfos(waitSemaphoreInfos);
    submitInfo.setSignalSemaphoreInfos(signalSemaphoreInfos);

    assert(renderSync.drawn);
    m_FrameInfo.device->SubmitToQueue(submitInfo, *renderSync.drawn);

    m_FrameInfo.sync->NextFrameIndex();
    m_RenderTarget.reset();
    vk::Queue queue = m_FrameInfo.device->GetQueue();


    const bool fbSizeChanged = m_FrameInfo.framebufferSize != m_FrameInfo.swapchain->GetSize();
    if (fbSizeChanged) {
        m_FrameInfo.swapchain->Recreate(m_FrameInfo.framebufferSize);
        return;
    }

    const bool outOfDate = !m_FrameInfo.swapchain->Present(queue);
    if (outOfDate) {
        m_FrameInfo.swapchain->Recreate(m_FrameInfo.framebufferSize);
        return;
    }
}
void APIRenderer::BindDescriptorSets(DescriptorBuffer &ubo, const vk::DescriptorImageInfo &descriptorImageInfo) {
    m_Backend->GetDescriptorSet().BindDescriptorSets(m_CommandBuffer, m_FrameInfo.sync->GetFrameIndex(),
                                 ubo.GetDescriptorInfoAt(m_FrameInfo.sync->GetFrameIndex()), descriptorImageInfo);
}


