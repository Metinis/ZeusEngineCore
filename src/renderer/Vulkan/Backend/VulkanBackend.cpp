#include "VulkanBackend.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <vulkan/vulkan.hpp>

VulkanBackend::VulkanBackend(const std::vector<const char*>& layers, WindowHandle windowHandle)
        : m_Instance(layers, GetRequiredExtensions()),

          m_WindowHandle(std::move(windowHandle)),

          m_Surface(CreateSurface(windowHandle, m_Instance.Get())),

          m_Device(m_Instance.Get(), m_Surface.get()),

          m_DebugMessenger(CreateMessenger(m_Instance.Get())),

          m_Waiter(m_Device.GetLogicalDevice()),

          m_Swapchain(CreateSwapchain(windowHandle, m_Device.GetLogicalDevice(), m_Device.GetGPU(), m_Surface.get())),

          m_Sync(m_Device.GetGPU(), m_Device.GetLogicalDevice()),

          m_Allocator(CreateMemoryAllocator(m_Instance.Get(), m_Device.GetGPU().device, m_Device.GetLogicalDevice()))

          {

    Init();
}

void VulkanBackend::Init()
{
    m_DeferredDestroyCallback = std::make_shared<std::function<void(BufferHandle)>>(
            [this](BufferHandle handle) {
                m_DeferredDestroy.push_back(handle);
            });

}

static constexpr auto vertexInput_v = VKShaderVertexInput{
        .attributes = vertexAttributes_v,
        .bindings = vertexBindings_v,
};
VulkanShaderInfo VulkanBackend::GetShaderInfo() const
{
    VulkanShaderInfo shaderInfo{};
    shaderInfo.device = m_Device.GetLogicalDevice();
    shaderInfo.vertexInput = vertexInput_v;
    return shaderInfo;
}
vk::UniqueSurfaceKHR VulkanBackend::CreateSurface(WindowHandle windowHandle, const vk::Instance instance) {
    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(windowHandle.nativeWindowHandle);

    VkSurfaceKHR ret{};
    auto const result =
            glfwCreateWindowSurface(instance, glfwWindow, nullptr, &ret);
    if (result != VK_SUCCESS || ret == VkSurfaceKHR{}) {
        throw std::runtime_error{"Failed to create Vulkan Surface"};
    }
    return vk::UniqueSurfaceKHR{ret, instance};
}

VulkanSwapchain VulkanBackend::CreateSwapchain(const WindowHandle windowHandle, const vk::Device device, const GPU& gpu,
    const vk::SurfaceKHR surface)
{
    int width;
    int height;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(windowHandle.nativeWindowHandle), &width, &height);
    return VulkanSwapchain(device, gpu, surface, glm::ivec2{width, height});
}

bool VulkanBackend::AcquireRenderTarget()
{
    int width;
    int height;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_WindowHandle.nativeWindowHandle), &width, &height);
    //minimized
    if (width <= 0 || height <= 0) {
        return false;
    }
    m_FramebufferSize = { width, height };

    RenderSync& renderSync = m_Sync.GetRenderSyncAtFrame();

    static constexpr std::uint64_t fence_timeout_v = 3'000'000'000ull; // 3 seconds in nanoseconds
    vk::Result result = m_Device.GetLogicalDevice().waitForFences(*renderSync.drawn, vk::True, fence_timeout_v);
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
void VulkanBackend::FlushDeferredDestroys() {
    for (auto& bufferHandle : m_DeferredDestroy) {
        vmaDestroyBuffer(bufferHandle.allocator, bufferHandle.buffer, bufferHandle.allocation);
    }
    m_DeferredDestroy.clear();
}
vk::CommandBuffer VulkanBackend::BeginFrame()
{
    FlushDeferredDestroys();

    RenderSync& renderSync = m_Sync.GetRenderSyncAtFrame();
    vk::CommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    renderSync.commandBuffer.begin(commandBufferBeginInfo);
    return renderSync.commandBuffer;
}

void VulkanBackend::TransitionForRender(vk::CommandBuffer const commandBuffer) const
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

void VulkanBackend::Render(vk::CommandBuffer const commandBuffer, const std::function<void(vk::CommandBuffer)>& drawCallback,
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

void VulkanBackend::TransitionForPresent(vk::CommandBuffer const commandBuffer) const
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

void VulkanBackend::SubmitAndPresent()
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


    const bool fb_size_changed = m_FramebufferSize != m_Swapchain.GetSize();
    if (fb_size_changed) {
        //m_Device.GetLogicalDevice().waitIdle();
        m_Swapchain.Recreate(m_FramebufferSize);
        return;
    }

    const bool out_of_date = !m_Swapchain.Present(queue);
    if (out_of_date) {
        //m_Device.GetLogicalDevice().waitIdle();
        m_Swapchain.Recreate(m_FramebufferSize);
        return;
    }
}

std::vector<const char*> VulkanBackend::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef __APPLE__
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifndef NDEBUG
    std::cout << "available extensions:\n";
    for (const auto& extension : vk::enumerateInstanceExtensionProperties()) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
#endif

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

vk::UniqueDebugUtilsMessengerEXT VulkanBackend::CreateMessenger(const vk::Instance instance) {
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

VulkanMemAlloc VulkanBackend::CreateMemoryAllocator(const vk::Instance instance, const vk::PhysicalDevice physicalDevice,
                                     const vk::Device logicalDevice) const {

    return VulkanMemAlloc(instance, physicalDevice, logicalDevice);
}

VulkanContextInfo VulkanBackend::GetContext()
{
    VulkanContextInfo contextInfo{};
    contextInfo.apiVersion = m_Instance.GetApiVersion();
    contextInfo.instance = m_Instance.Get();
    contextInfo.physicalDevice = m_Device.GetGPU().device;
    contextInfo.queueFamily = m_Device.GetGPU().queueFamily;
    contextInfo.device = m_Device.GetLogicalDevice();
    contextInfo.queue = m_Device.GetQueue();
    contextInfo.colorFormat = m_Swapchain.GetFormat();
    contextInfo.samples = vk::SampleCountFlagBits::e1;
    contextInfo.allocator = m_Allocator.Get();
    contextInfo.deferredDestroyBuffer = m_DeferredDestroyCallback;


    return contextInfo;
}

VulkanBackend::~VulkanBackend() {
    m_Device.GetLogicalDevice().waitIdle();
    FlushDeferredDestroys();
}


