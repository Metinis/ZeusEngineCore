#include "VulkanBackend.h"
#include "GLFW/glfw3.h"
#include <iostream>

VulkanBackend::VulkanBackend(const std::vector<const char*>& layers, WindowHandle* windowHandle)
        : m_Instance(layers, GetRequiredExtensions()),
          m_WindowHandle(windowHandle){

    m_DynamicLoader = vk::detail::DispatchLoaderDynamic(m_Instance.Get(), vkGetInstanceProcAddr);

    Init();
}

void VulkanBackend::Init()
{
    InitSurface();
    m_Device = std::make_unique<VulkanDevice>(m_Instance.Get(), m_Surface.get());
    InitMessenger();
}
void VulkanBackend::InitSurface() {
    if (m_WindowHandle) {
        GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(m_WindowHandle->nativeWindowHandle);

        VkSurfaceKHR rawSurface;
        VkResult result = glfwCreateWindowSurface(
            m_Instance.Get(),
            glfwWindow,
            nullptr,
            &rawSurface
        );

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan surface");
        }

        m_Surface = vk::UniqueHandle<vk::SurfaceKHR, vk::detail::DispatchLoaderDynamic>(
            rawSurface,
            vk::detail::ObjectDestroy<vk::Instance, vk::detail::DispatchLoaderDynamic>(
                m_Instance.Get(),
                nullptr,
                m_DynamicLoader
            )
        );
    }
}

std::vector<const char*> VulkanBackend::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef __APPLE__
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif


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