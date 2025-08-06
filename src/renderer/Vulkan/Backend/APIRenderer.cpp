#include "APIRenderer.h"
#include "../DescriptorBuffer.h"
#include "ZeusEngineCore/Vertex.h"
#include "../Texture.h"

using namespace ZEN::VKAPI;

APIRenderer::APIRenderer(APIBackend* apiBackend) : m_Backend(apiBackend){
    m_Samples = vk::SampleCountFlagBits::e4;
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
//constexpr vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e4; //todo use actual count
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
    if(!m_ColorImage.has_value() || !m_ColorImageView.has_value() || m_ColorImage.value().GetSampleCount() != m_Samples) {
        //if msaa changes, wait for previous frame to fully finish by waiting idle
        m_FrameInfo.device->GetLogicalDevice().waitIdle();
        m_ColorImage.emplace(m_Backend->CreateColorImage(m_RenderTarget->extent, m_Samples));
        m_ColorImageView.emplace(m_Backend->CreateColorImageView(m_ColorImage->Get()));
    }

    if(!m_MSAADepthImage.has_value() || !m_MSAADepthImageView.has_value() || m_MSAADepthImage.value().GetSampleCount() != m_Samples) {
        m_FrameInfo.device->GetLogicalDevice().waitIdle();
        m_MSAADepthImage.emplace(m_Backend->CreateDepthImage(m_RenderTarget->extent, m_Samples));
        m_MSAADepthImageView.emplace(m_Backend->CreateDepthImageView(m_MSAADepthImage->Get()));
    }
    
    m_FrameInfo.device->GetLogicalDevice().resetFences(*renderSync.drawn);

    return true;
}
void APIRenderer::TransitionForRender(){
    SetBarriersForRender();
    //no depth by default
    SetAttachments(false, false, false);
    
    
}

void APIRenderer::TransitionForPresent(){
    m_CommandBuffer.endRendering();

    SetBarriersForPresent();
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
        //use framebuffer size since m_RenderTarget is not updated
        vk::Extent2D extent;
        extent.width = m_FrameInfo.framebufferSize.x;
        extent.height = m_FrameInfo.framebufferSize.y;
        //recreate msaa depth image
        m_MSAADepthImage.emplace(m_Backend->CreateDepthImage(extent, m_Samples));
        m_MSAADepthImageView.emplace(m_Backend->CreateDepthImageView(m_MSAADepthImage->Get()));
        //recreate color image
        m_ColorImage.emplace(m_Backend->CreateColorImage(extent, m_Samples));
        m_ColorImageView.emplace(m_Backend->CreateColorImageView(m_ColorImage->Get()));

        return;
    }

    const bool outOfDate = !m_FrameInfo.swapchain->Present(queue);
    if (outOfDate) {
        m_FrameInfo.swapchain->Recreate(m_FrameInfo.framebufferSize);
        return;
    }
}


void APIRenderer::DrawWithCallback(const std::function<void(void*)>& uiExtraDrawCallback) {
    m_CommandBuffer.endRendering();
    SetAttachments(false, false, false);
    if(uiExtraDrawCallback){
        uiExtraDrawCallback(static_cast<void*>(m_CommandBuffer));
    }
}



void APIRenderer::SetBarriersForRender() const {
    // --- Existing color attachment barrier ---
    auto colorBarrier = m_FrameInfo.swapchain->GetBaseBarrier();
    colorBarrier
            .setOldLayout(vk::ImageLayout::eUndefined)
            .setNewLayout(vk::ImageLayout::eAttachmentOptimal)
            .setSrcAccessMask({})
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
            .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
            .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    // --- New depth barrier ---
    //m_NewLayout = vk::ImageLayout::eDepthAttachmentOptimal;
    vk::ImageMemoryBarrier2 depthBarrier{
            vk::PipelineStageFlagBits2::eEarlyFragmentTests,  // Wait for prior depth writes
            vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            vk::PipelineStageFlagBits2::eEarlyFragmentTests,  // Before depth tests/writes
            vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            vk::ImageLayout::eUndefined,                     // Previous layout
            vk::ImageLayout::eDepthAttachmentOptimal,        // New layout
            m_Backend->GetQueueFamily(),
            m_Backend->GetQueueFamily(),
            m_MSAADepthImage->Get(),                             // Depth image
            vk::ImageSubresourceRange(
                    vk::ImageAspectFlagBits::eDepth,             // Depth aspect only
                    0, 1, 0, 1                                   // All mips/layers
            )
    };

    // Submit both barriers together
    std::array barriers{colorBarrier, depthBarrier};
    vk::DependencyInfo dependencyInfo{};
    dependencyInfo.setImageMemoryBarriers(barriers);
    m_CommandBuffer.pipelineBarrier2(dependencyInfo);
}

void APIRenderer::SetBarriersForPresent() const {
    // --- Existing color barrier ---
    auto colorBarrier = m_FrameInfo.swapchain->GetBaseBarrier();
    colorBarrier
            .setOldLayout(vk::ImageLayout::eAttachmentOptimal)
            .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
            .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .setDstAccessMask(colorBarrier.srcAccessMask)
            .setDstStageMask(colorBarrier.srcStageMask);

    // --- New depth barrier ---
    // m_NewLayout = vk::ImageLayout::eReadOnlyOptimal;
    vk::ImageMemoryBarrier2 depthBarrier{
            vk::PipelineStageFlagBits2::eLateFragmentTests,
            vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            vk::PipelineStageFlagBits2::eFragmentShader,
            vk::AccessFlagBits2::eShaderRead,
            vk::ImageLayout::eDepthAttachmentOptimal,
            vk::ImageLayout::eReadOnlyOptimal,
            m_Backend->GetQueueFamily(),
            m_Backend->GetQueueFamily(),
            m_MSAADepthImage->Get(),
            vk::ImageSubresourceRange(
                    vk::ImageAspectFlagBits::eDepth,
                    0, 1, 0, 1
            )
    };
    //m_OldLayout = m_NewLayout;

    std::array barriers{colorBarrier, depthBarrier};
    vk::DependencyInfo dependencyInfo{};
    dependencyInfo.setImageMemoryBarriers(barriers);
    m_CommandBuffer.pipelineBarrier2(dependencyInfo);
}

void APIRenderer::SetAttachments(const bool shouldClearColor, const bool shouldClearDepth,
                                 const bool shouldUseDepth) {
    //end rendering before setting new attachments
    //m_CommandBuffer.endRendering();

    vk::RenderingAttachmentInfo colorAttachment{};

    colorAttachment.imageView = m_RenderTarget.value().imageView;
    colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.loadOp = shouldClearColor ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f };


    if(m_Samples != vk::SampleCountFlagBits::e1) {
        colorAttachment.imageView = m_ColorImageView.value().get();
        colorAttachment.resolveMode = vk::ResolveModeFlagBits::eAverage;
        colorAttachment.resolveImageView = m_RenderTarget.value().imageView;
        colorAttachment.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    }

    vk::Rect2D renderArea = { vk::Offset2D{}, m_RenderTarget->extent };

    vk::RenderingInfo renderingInfo{};
    renderingInfo = vk::RenderingInfo{};
    renderingInfo.setRenderArea(renderArea)
            .setLayerCount(1)
            .setColorAttachments(colorAttachment)
            .setPDepthAttachment(nullptr);


    if(shouldUseDepth) {
        vk::RenderingAttachmentInfo depthAttachment{};
        depthAttachment.imageView = *m_MSAADepthImageView.value(); //todo use another depth buffer in future
        depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        depthAttachment.loadOp = shouldClearDepth ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        depthAttachment.clearValue = vk::ClearDepthStencilValue{1.0f, 0};
        renderingInfo.setPDepthAttachment(&depthAttachment);

    }

    m_CommandBuffer.beginRendering(renderingInfo);
    m_CommandBuffer.setRasterizationSamplesEXT(m_Samples);
    
}

void APIRenderer::Clear(const bool shouldClearColor, const bool shouldClearDepth) {
    m_CommandBuffer.endRendering();
    SetAttachments(shouldClearColor, shouldClearDepth, true);
}
void APIRenderer::SetUBO(const DescriptorBuffer& ubo) {
    vk::DescriptorBufferInfo info = ubo.GetDescriptorInfoAt(GetFrameIndex());
    m_Backend->GetDescriptorSet().SetUBO(GetFrameIndex(), info);
}
void APIRenderer::SetSSBO(const DescriptorBuffer& ubo){
    vk::DescriptorBufferInfo info = ubo.GetDescriptorInfoAt(GetFrameIndex());
    m_Backend->GetDescriptorSet().SetSSBO(GetFrameIndex(), info);
}

void APIRenderer::SetImage(const vk::DescriptorImageInfo& imageInfo) {
    m_Backend->GetDescriptorSet().SetImage(GetFrameIndex(), imageInfo);
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
            .setHeight(-static_cast<float>(extent.height))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

    m_CommandBuffer.setViewport(0, viewport);
    m_CommandBuffer.setScissor(0, vk::Rect2D{{}, extent});
}

void APIRenderer::DrawIndexed(const DrawIndexedInfo& drawInfo, const std::uint32_t instanceCount) const {
    m_CommandBuffer.bindVertexBuffers(drawInfo.firstBinding, drawInfo.buffer, drawInfo.vertexOffset);

    m_CommandBuffer.bindIndexBuffer(drawInfo.buffer, drawInfo.indexOffset,
                                    drawInfo.type);
    m_CommandBuffer.drawIndexed(drawInfo.indexCount, instanceCount,
                                drawInfo.firstIndex, drawInfo.vertexOffsetInIndices, 0);
}

void APIRenderer::SetDepth(bool isDepth) {
    m_CommandBuffer.endRendering();
    SetAttachments(false, false, isDepth);
}
constexpr vk::SampleCountFlagBits toSampleCountFlagBits(int sampleCount) {
    switch (sampleCount) {
        case 1:  return vk::SampleCountFlagBits::e1;
        case 2:  return vk::SampleCountFlagBits::e2;
        case 3:  return vk::SampleCountFlagBits::e4;
        case 4:  return vk::SampleCountFlagBits::e8;
        case 5: return vk::SampleCountFlagBits::e16;
        case 6: return vk::SampleCountFlagBits::e32;
        case 7: return vk::SampleCountFlagBits::e64;
        default: return vk::SampleCountFlagBits::e1;
    }
}
void APIRenderer::SetMSAA(int msaa) {
    assert(msaa > 0);
    m_Samples = toSampleCountFlagBits(msaa);
}

int APIRenderer::GetMaxMSAA() {

    vk::SampleCountFlags counts = m_FrameInfo.device->GetGPU().properties.limits.framebufferColorSampleCounts &
        m_FrameInfo.device->GetGPU().properties.limits.framebufferDepthSampleCounts;
    if (counts & vk::SampleCountFlagBits::e64) { return 7; }
    if (counts & vk::SampleCountFlagBits::e32) { return 6; }
    if (counts & vk::SampleCountFlagBits::e16) { return 5; }
    if (counts & vk::SampleCountFlagBits::e8) { return 4; }
    if (counts & vk::SampleCountFlagBits::e4) { return 3; }
    if (counts & vk::SampleCountFlagBits::e2) { return 2; }
    if (counts & vk::SampleCountFlagBits::e1) { return 1; }

    return 0;
}


