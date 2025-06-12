#pragma once
#include <vulkan/vulkan.hpp>
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
    vk::detail::DispatchLoaderDynamic m_DynamicLoader;
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::detail::DispatchLoaderDynamic> m_DebugMessenger;
    std::shared_ptr<VulkanDevice> m_Device;
    vk::UniqueHandle<vk::SurfaceKHR, vk::detail::DispatchLoaderDynamic> m_Surface;
    WindowHandle* m_WindowHandle = nullptr;
};
