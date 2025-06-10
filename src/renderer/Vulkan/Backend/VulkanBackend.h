#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanInstance.h"
#include "VulkanDevice.h"

class VulkanBackend {
public:
    VulkanBackend(const std::vector<const char*>& layers);
    ~VulkanBackend() = default;
    void Init();
private:
    std::vector<const char*> GetRequiredExtensions();
    void InitMessenger();

    VulkanInstance m_Instance;
    vk::detail::DispatchLoaderDynamic m_DynamicLoader;
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::detail::DispatchLoaderDynamic> m_DebugMessenger;
    VulkanDevice m_Device;
};
