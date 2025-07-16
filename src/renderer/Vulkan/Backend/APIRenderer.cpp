#include "APIRenderer.h"
#include "DescriptorBuffer.h"
#include "ZeusEngineCore/Vertex.h"
#include "../Texture.h"

using namespace ZEN::VKAPI;

APIRenderer::APIRenderer(APIBackend* apiBackend) : m_Backend(apiBackend){

}
bool APIRenderer::BeginFrame(){
    if(!AcquireRenderTarget())
        return false;

    m_Backend->FlushDeferredDestroys();
    RenderSync& renderSync = m_FrameInfo.sync->GetRenderSyncAtFrame();
    vk::CommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    renderSync.commandBuffer.begin(commandBufferBeginInfo);
    m_CommandBuffer = renderSync.commandBuffer;

    TransitionForRender();


    return true;
}
bool APIRenderer::AcquireRenderTarget(){
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
    m_RenderTarget = m_FrameInfo.swapchain->AcquireNextImage(*renderSync.draw);
    if (!m_RenderTarget) {
        // acquire failure
        m_FrameInfo.swapchain->Recreate(m_FrameInfo.framebufferSize);
        return false;
    }

    m_FrameInfo.device->GetLogicalDevice().resetFences(*renderSync.drawn);

    return true;
}


void APIRenderer::TransitionForRender(){
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

    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = m_RenderTarget->imageView;
    colorAttachment.imageLayout = vk::ImageLayout::eAttachmentOptimal;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue = vk::ClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f };

    auto const renderArea = vk::Rect2D{ vk::Offset2D{}, m_RenderTarget->extent };

    m_RenderingInfo.renderArea = renderArea;
    m_RenderingInfo.setColorAttachments(colorAttachment);
    m_RenderingInfo.layerCount = 1;

    m_CommandBuffer.beginRendering(m_RenderingInfo);
}

void APIRenderer::TransitionForPresent() const{
    m_CommandBuffer.endRendering();
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
    TransitionForPresent();
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

void APIRenderer::DrawWithCallback(const std::function<void(void*)>& uiExtraDrawCallback) {
    if(uiExtraDrawCallback){
        uiExtraDrawCallback(static_cast<void*>(m_CommandBuffer));
    }

}

void APIRenderer::SetUBO(const DescriptorBuffer& ubo) {
    vk::DescriptorBufferInfo info = ubo.GetDescriptorInfoAt(m_FrameInfo.sync->GetFrameIndex());
    m_Backend->GetDescriptorSet().SetUBO(GetFrameIndex(), info);
}

void APIRenderer::SetImage(const Texture& texture) {
    m_Backend->GetDescriptorSet().SetImage(GetFrameIndex(),
                                           texture.GetDescriptorInfo());
}

void APIRenderer::BindDescriptorSets() {
    m_Backend->GetDescriptorSet().BindDescriptorSets(m_CommandBuffer,
                                                     GetFrameIndex());
}

void APIRenderer::BindShader(vk::Pipeline pipeline) {
    vk::Extent2D extent = m_RenderTarget->extent;
    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    vk::Viewport viewport{};
    viewport.setX(0.0f)
            .setY(static_cast<float>(extent.height))
            .setWidth(static_cast<float>(extent.width))
            .setHeight(-viewport.y);
    m_CommandBuffer.setViewport(0, viewport);
    m_CommandBuffer.setScissor(0, vk::Rect2D{{}, extent});
}

void APIRenderer::DrawIndexed(vk::Buffer buffer) const {
    //todo send in offset data etc
    m_CommandBuffer.bindVertexBuffers(0, buffer, vk::DeviceSize{});

    m_CommandBuffer.bindIndexBuffer(buffer, 4 * sizeof(Vertex),
                                  vk::IndexType::eUint32);
    m_CommandBuffer.drawIndexed(6, 1, 0, 0, 0);
}


