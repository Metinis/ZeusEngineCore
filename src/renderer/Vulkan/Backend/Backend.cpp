#include "Backend.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

using namespace ZEN::VKAPI;

Backend::Backend(WindowHandle windowHandle)
    : m_Instance(),

    m_WindowHandle(windowHandle),

    m_Surface(CreateSurface(m_WindowHandle, m_Instance.Get())),

    m_Device(m_Instance.Get(), m_Surface.get()),

    m_DebugMessenger(CreateMessenger(m_Instance.Get())),

    m_Waiter(m_Device.GetLogicalDevice()),

    m_Swapchain(CreateSwapchain(m_WindowHandle, m_Device.GetLogicalDevice(), m_Device.GetGPU(), m_Surface.get())),

    m_Sync(m_Device.GetGPU(), m_Device.GetLogicalDevice()),

    m_Allocator(CreateMemoryAllocator(m_Instance.Get(), m_Device.GetGPU().device, m_Device.GetLogicalDevice())),

    m_CommandBlockPool(CreateCommandBlockPool()),

    m_DescSet(DescriptorSet(m_Device.GetLogicalDevice()))

    {

    Init();
}

void Backend::Init()
{
    m_DeferredDestroyCallback = std::make_shared<std::function<void(DeferredHandle)>>(
            [this](DeferredHandle handle) {
                m_DeferredDestroy.push_back(handle);
            });

}
ShaderInfo Backend::GetShaderInfo() const
{
    ShaderInfo shaderInfo{};
    shaderInfo.device = m_Device.GetLogicalDevice();
    shaderInfo.colorFormat = m_Swapchain.GetFormat();
    shaderInfo.samples = vk::SampleCountFlagBits::e1;
    shaderInfo.pipelineLayout = m_DescSet.GetPipelineLayout();
    return shaderInfo;
}
TextureInfo Backend::GetTextureInfo()
{
    TextureInfo textureInfo{};
    textureInfo.allocator = m_Allocator.Get();
    textureInfo.commandBlock.emplace(CommandBlock(m_Device.GetLogicalDevice(),
                                                  m_Device.GetQueue(), m_CommandBlockPool.get()));
    textureInfo.device = m_Device.GetLogicalDevice();
    textureInfo.queueFamily = m_Device.GetGPU().queueFamily;
    textureInfo.sampler.setMagFilter(vk::Filter::eNearest);
    textureInfo.destroyCallback = m_DeferredDestroyCallback;
    return textureInfo;
    
}
vk::UniqueSurfaceKHR Backend::CreateSurface(WindowHandle windowHandle, const vk::Instance instance) {
    auto* glfwWindow = windowHandle.nativeWindowHandle;

    VkSurfaceKHR ret{};
    auto const result =
            glfwCreateWindowSurface(instance, glfwWindow, nullptr, &ret);
    if (result != VK_SUCCESS || ret == VkSurfaceKHR{}) {
        throw std::runtime_error{"Failed to create Vulkan Surface"};
    }
    return vk::UniqueSurfaceKHR{ret, instance};
}

Swapchain Backend::CreateSwapchain(const WindowHandle windowHandle, const vk::Device device, const GPU& gpu,
                                   const vk::SurfaceKHR surface)
{
    int width;
    int height;
    glfwGetFramebufferSize(windowHandle.nativeWindowHandle, &width, &height);
    return Swapchain(device, gpu, surface, glm::ivec2{width, height});
}

bool Backend::AcquireRenderTarget()
{
    int width;
    int height;
    glfwGetFramebufferSize(m_WindowHandle.nativeWindowHandle, &width, &height);
    //minimized
    if (width <= 0 || height <= 0) {
        return false;
    }
    m_FramebufferSize = { width, height };

    RenderSync& renderSync = m_Sync.GetRenderSyncAtFrame();

    static constexpr std::uint64_t fenceTimeout_v = 3'000'000'000ull; // 3 seconds in nanoseconds
    vk::Result result = m_Device.GetLogicalDevice().waitForFences(*renderSync.drawn, vk::True, fenceTimeout_v);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error{ "Failed to wait for Render Fence" };
    }
    m_RenderTarget = m_Swapchain.AquireNextImage(*renderSync.draw);
    if (!m_RenderTarget) {
        // acquire failure
        m_Swapchain.Recreate(m_FramebufferSize);
        return false;
    }

    m_Device.GetLogicalDevice().resetFences(*renderSync.drawn);

    return true;
}
void Backend::FlushDeferredDestroys() {
    for (auto& handle : m_DeferredDestroy) {
        if(std::holds_alternative<BufferHandle>(handle)){
            const auto& h = std::get<BufferHandle>(handle);
            vmaDestroyBuffer(h.allocator, h.buffer, h.allocation);
        }
        else if(std::holds_alternative<ImageHandle>(handle)){
            const auto& h = std::get<ImageHandle>(handle);
            vmaDestroyImage(h.allocator, h.image, h.allocation);
        }
    }
    m_DeferredDestroy.clear();
}
vk::CommandBuffer Backend::BeginFrame()
{
    FlushDeferredDestroys();

    RenderSync& renderSync = m_Sync.GetRenderSyncAtFrame();
    vk::CommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    renderSync.commandBuffer.begin(commandBufferBeginInfo);
    return renderSync.commandBuffer;
}

void Backend::TransitionForRender(vk::CommandBuffer const commandBuffer) const
{
    auto barrier = m_Swapchain.GetBaseBarrier();
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

    commandBuffer.pipelineBarrier2(dependency_info);
}

void Backend::Render(vk::CommandBuffer const commandBuffer, const std::function<void(vk::CommandBuffer)>& drawCallback,
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

    commandBuffer.beginRendering(renderingInfo);
    //draw mesh stuff here
    if(drawCallback) drawCallback(commandBuffer);
    commandBuffer.endRendering();

    // UI pass
    colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
    renderingInfo.setColorAttachments(colorAttachment);
    renderingInfo.setPDepthAttachment(nullptr); // no depth

    commandBuffer.beginRendering(renderingInfo);
    if(uiDrawCallback) uiDrawCallback(commandBuffer);
    commandBuffer.endRendering();
}

void Backend::TransitionForPresent(vk::CommandBuffer const commandBuffer) const
{
    vk::DependencyInfo dependencyInfo{};
    auto barrier = m_Swapchain.GetBaseBarrier();

    barrier.oldLayout = vk::ImageLayout::eAttachmentOptimal;
    barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    barrier.dstAccessMask = barrier.srcAccessMask;
    barrier.dstStageMask = barrier.srcStageMask;

    dependencyInfo.setImageMemoryBarriers(barrier);
    commandBuffer.pipelineBarrier2(dependencyInfo);
}

void Backend::SubmitAndPresent()
{
    RenderSync& renderSync = m_Sync.GetRenderSyncAtFrame();
    renderSync.commandBuffer.end();

    vk::SubmitInfo2 submitInfo{};

    vk::CommandBufferSubmitInfo cmdBuffSubmitInfo{ renderSync.commandBuffer };

    vk::SemaphoreSubmitInfo waitSemaphoreInfo{};
    waitSemaphoreInfo.semaphore = *renderSync.draw;
    waitSemaphoreInfo.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

    vk::SemaphoreSubmitInfo signalSemaphoreInfo{};
    signalSemaphoreInfo.semaphore = m_Swapchain.GetPresentSemaphore();
    signalSemaphoreInfo.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

    std::array<vk::CommandBufferSubmitInfo, 1> cmdBuffInfos{ cmdBuffSubmitInfo };
    std::array<vk::SemaphoreSubmitInfo, 1> waitSemaphoreInfos{ waitSemaphoreInfo };
    std::array<vk::SemaphoreSubmitInfo, 1> signalSemaphoreInfos{ signalSemaphoreInfo };

    submitInfo.setCommandBufferInfos(cmdBuffInfos);
    submitInfo.setWaitSemaphoreInfos(waitSemaphoreInfos);
    submitInfo.setSignalSemaphoreInfos(signalSemaphoreInfos);

    assert(renderSync.drawn);
    m_Device.SubmitToQueue(submitInfo, *renderSync.drawn);

    m_Sync.NextFrameIndex();
    m_RenderTarget.reset();
    vk::Queue queue = m_Device.GetQueue();


    const bool fbSizeChanged = m_FramebufferSize != m_Swapchain.GetSize();
    if (fbSizeChanged) {
        m_Swapchain.Recreate(m_FramebufferSize);
        return;
    }

    const bool outOfDate = !m_Swapchain.Present(queue);
    if (outOfDate) {
        m_Swapchain.Recreate(m_FramebufferSize);
        return;
    }
}

DescriptorBuffer Backend::CreateUBO() const
{
    BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.allocator = m_Allocator.Get();
    bufferCreateInfo.queueFamily = m_Device.GetGPU().queueFamily;
    bufferCreateInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
    bufferCreateInfo.destroyCallback = m_DeferredDestroyCallback;
    return DescriptorBuffer(bufferCreateInfo);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

vk::UniqueDebugUtilsMessengerEXT Backend::CreateMessenger(const vk::Instance instance) {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfo.pfnUserCallback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback);
    createInfo.pUserData = nullptr;


    return instance.createDebugUtilsMessengerEXTUnique(createInfo);
}

MemAllocator Backend::CreateMemoryAllocator(const vk::Instance instance, const vk::PhysicalDevice physicalDevice,
                                            const vk::Device logicalDevice) const {

    return MemAllocator(instance, physicalDevice, logicalDevice);
}

vk::UniqueCommandPool Backend::CreateCommandBlockPool() const
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.setQueueFamilyIndex(m_Device.GetGPU().queueFamily);
    commandPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient);
    return m_Device.GetLogicalDevice().createCommandPoolUnique(commandPoolCreateInfo);

}

ContextInfo Backend::GetContext()
{
    ContextInfo contextInfo{};
    contextInfo.apiVersion = m_Instance.GetApiVersion();
    contextInfo.instance = m_Instance.Get();
    contextInfo.physicalDevice = m_Device.GetGPU().device;
    contextInfo.queueFamily = m_Device.GetGPU().queueFamily;
    contextInfo.device = m_Device.GetLogicalDevice();
    contextInfo.queue = m_Device.GetQueue();
    contextInfo.colorFormat = m_Swapchain.GetFormat();
    contextInfo.samples = vk::SampleCountFlagBits::e1;
    contextInfo.allocator = m_Allocator.Get();
    contextInfo.commandBlockPool = m_CommandBlockPool.get();
    contextInfo.destroyCallback = m_DeferredDestroyCallback;


    return contextInfo;
}

Backend::~Backend() {
    m_Device.GetLogicalDevice().waitIdle();
    FlushDeferredDestroys();
}



