#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "../../../Utils.h"
class VulkanBackend {
public:
    VulkanBackend(const std::vector<const char*>& layers, WindowHandle* windowHandle);
    ~VulkanBackend() = default;
    void Init();
private:
    std::vector<const char*> GetRequiredExtensions();
    void InitMessenger();
    void InitSurface();
    VulkanInstance m_Instance;
    vk::DispatchLoaderDynamic m_DynamicLoader;
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> m_DebugMessenger;
    std::shared_ptr<VulkanDevice> m_Device;
    vk::UniqueHandle<vk::SurfaceKHR, vk::DispatchLoaderDynamic> m_Surface;
    WindowHandle* m_WindowHandle = nullptr;
};
