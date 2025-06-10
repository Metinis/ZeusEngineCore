#include "VulkanBackend.h"
#include "GLFW/glfw3.h"
#include <iostream>

VulkanBackend::VulkanBackend(const std::vector<const char*>& layers)
        : m_Instance(layers, GetRequiredExtensions()),
          m_Device(m_Instance.Get())
{
    // Initialize default dispatcher
    //VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    // Initialize instance-specific dispatcher
    m_DynamicLoader = vk::detail::DispatchLoaderDynamic(m_Instance.Get(), vkGetInstanceProcAddr);

    Init();
}

void VulkanBackend::Init()
{
    InitMessenger();
}

std::vector<const char*> VulkanBackend::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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

void VulkanBackend::InitMessenger() {
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


    m_DebugMessenger = m_Instance.Get().createDebugUtilsMessengerEXTUnique(
            createInfo,
            nullptr,
            m_DynamicLoader
    );
}