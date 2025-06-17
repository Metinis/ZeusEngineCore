#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "../../../Utils.h"
#include "ScopedWaiter.h"
#include "VulkanSwapchain.h"

class VulkanBackend {
public:
    VulkanBackend(const std::vector<const char*>& layers, WindowHandle* windowHandle);
    ~VulkanBackend() = default;
    void Init();
private:
    using VulkanDebugMessenger = vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::detail::DispatchLoaderDynamic>;
    //using VulkanSurface = vk::UniqueHandle<vk::SurfaceKHR, vk::detail::DispatchLoaderDynamic>;
    

    std::vector<const char*> GetRequiredExtensions();
    VulkanDebugMessenger CreateMessenger(const vk::Instance instance, const DispatchLoaderDynamic& loader);
    vk::UniqueSurfaceKHR CreateSurface(WindowHandle* windowHandle, const vk::Instance instance);
    VulkanSwapchain CreateSwapchain(const WindowHandle* windowHandle, const vk::Device device, 
        const GPU& gpu, const vk::SurfaceKHR surface, const DispatchLoaderDynamic& loader);

    //order matters
    VulkanInstance m_Instance;
    DispatchLoaderDynamic m_DynamicLoader;
    WindowHandle* m_WindowHandle = nullptr;
    vk::UniqueSurfaceKHR m_Surface;
    VulkanDevice m_Device;
    VulkanDebugMessenger m_DebugMessenger;
    ScopedWaiter m_Waiter;
    VulkanSwapchain m_Swapchain;
    
};
