#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
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
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> m_DebugMessenger;
    VulkanDevice m_Device;
};
