#include "APIBackend.h"
#include "GLFW/glfw3.h"
#include "../Mesh.h"
#include "../Texture.h"
#include <iostream>
#include <vulkan/vulkan.hpp>
#include "ZeusEngineCore/IDescriptorBuffer.h"
#include "../ShaderPipeline.h"

using namespace ZEN::VKAPI;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;
APIBackend::APIBackend(WindowHandle windowHandle)
    : m_Instance(),

    m_WindowHandle(windowHandle),

    m_Surface(CreateSurface(m_WindowHandle, m_Instance.Get())),

    m_Device(m_Instance.Get(), m_Surface.get()),

#ifndef NDEBUG
    m_DebugMessenger(CreateMessenger(m_Instance.Get())),
#endif

    m_Waiter(m_Device.GetLogicalDevice()),

    m_Swapchain(CreateSwapchain(m_WindowHandle, m_Device.GetLogicalDevice(), m_Device.GetGPU(), m_Surface.get())),

    m_Sync(m_Device.GetGPU(), m_Device.GetLogicalDevice()),

    m_Allocator(CreateMemoryAllocator(m_Instance.Get(), m_Device.GetGPU().device, m_Device.GetLogicalDevice())),

    m_CommandBlockPool(CreateCommandBlockPool()),

    m_DescSet(DescriptorSet(m_Device.GetLogicalDevice()))

    {

    Init();
}

void APIBackend::Init()
{
    m_DeferredDestroyCallback = std::make_shared<std::function<void(DeferredHandle)>>(
            [this](DeferredHandle handle) {
                m_DeferredDestroy.push_back(handle);
            });

}
ShaderInfo APIBackend::GetShaderInfo() const
{
    ShaderInfo shaderInfo{};
    shaderInfo.device = m_Device.GetLogicalDevice();
    shaderInfo.colorFormat = m_Swapchain.GetFormat();
    shaderInfo.samples = vk::SampleCountFlagBits::e1;
    shaderInfo.pipelineLayout = m_DescSet.GetPipelineLayout();
    return shaderInfo;
}
TextureInfo APIBackend::GetTextureInfo()
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
vk::UniqueSurfaceKHR APIBackend::CreateSurface(WindowHandle windowHandle, const vk::Instance instance) {
    auto* glfwWindow = windowHandle.nativeWindowHandle;

    VkSurfaceKHR ret{};
    auto const result =
            glfwCreateWindowSurface(instance, glfwWindow, nullptr, &ret);
    if (result != VK_SUCCESS || ret == VkSurfaceKHR{}) {
        throw std::runtime_error{"Failed to create Vulkan Surface"};
    }
    return vk::UniqueSurfaceKHR{ret, instance};
}

Swapchain APIBackend::CreateSwapchain(const WindowHandle windowHandle, const vk::Device device, const GPU& gpu,
                                      const vk::SurfaceKHR surface)
{
    int width;
    int height;
    glfwGetFramebufferSize(windowHandle.nativeWindowHandle, &width, &height);
    return Swapchain(device, gpu, surface, glm::ivec2{width, height});
}

void APIBackend::FlushDeferredDestroys() {
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

BufferCreateInfo APIBackend::GetBufferCreateInfo(const ZEN::eDescriptorBufferType type) const
{
    BufferCreateInfo ret{};
    ret.allocator = m_Allocator.Get();
    ret.queueFamily = m_Device.GetGPU().queueFamily;

    if(type == ZEN::eDescriptorBufferType::UBO)
        ret.usage = vk::BufferUsageFlagBits::eUniformBuffer;

    else if(type == ZEN::eDescriptorBufferType::SSBO)
        ret.usage = vk::BufferUsageFlagBits::eStorageBuffer;

    ret.destroyCallback = m_DeferredDestroyCallback;
    return ret;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

vk::UniqueDebugUtilsMessengerEXT APIBackend::CreateMessenger(const vk::Instance instance) {
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

MemAllocator APIBackend::CreateMemoryAllocator(const vk::Instance instance, const vk::PhysicalDevice physicalDevice,
                                               const vk::Device logicalDevice) const {

    return MemAllocator(instance, physicalDevice, logicalDevice);
}

vk::UniqueCommandPool APIBackend::CreateCommandBlockPool() const
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.setQueueFamilyIndex(m_Device.GetGPU().queueFamily);
    commandPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient);
    return m_Device.GetLogicalDevice().createCommandPoolUnique(commandPoolCreateInfo);

}

APIBackend::~APIBackend() {
    m_Device.GetLogicalDevice().waitIdle();
    FlushDeferredDestroys();
}

glm::ivec2 APIBackend::GetFramebufferSize() const {
    int width;
    int height;
    glfwGetFramebufferSize(m_WindowHandle.nativeWindowHandle, &width, &height);
    return {width, height};
}

RenderFrameInfo APIBackend::GetRenderFrameInfo() {
    RenderFrameInfo ret{
        .framebufferSize = GetFramebufferSize(),
        .sync = &m_Sync,
        .device = &m_Device,
        .swapchain = &m_Swapchain,
    };
    return ret;
}

MeshInfo APIBackend::GetMeshInfo() const {
    MeshInfo ret{};
    ret.device = m_Device.GetLogicalDevice();
    ret.allocator = m_Allocator.Get();
    ret.destroyCallback = m_DeferredDestroyCallback;
    ret.queueFamily = m_Device.GetGPU().queueFamily;
    ret.queue = m_Device.GetQueue();
    ret.commandBlockPool = *m_CommandBlockPool;
    return ret;
}

BackendInfo APIBackend::GetInfo() const {
    BackendInfo ret{};
    ret.physicalDevice = m_Device.GetGPU().device;
    ret.queue = m_Device.GetQueue();
    ret.device = m_Device.GetLogicalDevice();
    ret.queueFamily = m_Device.GetGPU().queueFamily;
    ret.instance = m_Instance.Get();
    ret.colorFormat = m_Swapchain.GetFormat();
    ret.samples = vk::SampleCountFlagBits::e1;
    ret.apiVersion = m_Instance.GetApiVersion();
    return ret;
}

ZEN::eRendererAPI APIBackend::GetAPI() const {
    return ZEN::eRendererAPI::Vulkan;
}



