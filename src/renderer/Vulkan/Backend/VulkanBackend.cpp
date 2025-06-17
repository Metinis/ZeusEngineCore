#include "VulkanBackend.h"
#include "GLFW/glfw3.h"
#include <iostream>

VulkanBackend::VulkanBackend(const std::vector<const char*>& layers, WindowHandle* windowHandle)
        : m_Instance(layers, GetRequiredExtensions()),

          m_DynamicLoader(DispatchLoaderDynamic(m_Instance.Get(), vkGetInstanceProcAddr)),

          m_WindowHandle(windowHandle),

          m_Surface(CreateSurface(windowHandle, m_Instance.Get())),

          m_Device(m_Instance.Get(), m_Surface.get(), m_DynamicLoader),

          m_DebugMessenger(CreateMessenger(m_Instance.Get(), m_DynamicLoader)),

          m_Waiter(m_Device.getLogicalDevice()),

          m_Swapchain(CreateSwapchain(windowHandle, m_Device.getLogicalDevice(), m_Device.getGPU(), m_Surface.get(), m_DynamicLoader))

          {

    Init();
}

void VulkanBackend::Init()
{

}
vk::UniqueSurfaceKHR VulkanBackend::CreateSurface(WindowHandle* windowHandle, const vk::Instance instance) {
    if (windowHandle) {
        GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(windowHandle->nativeWindowHandle);

        VkSurfaceKHR ret{};
        auto const result =
                glfwCreateWindowSurface(instance, glfwWindow, nullptr, &ret);
        if (result != VK_SUCCESS || ret == VkSurfaceKHR{}) {
            throw std::runtime_error{"Failed to create Vulkan Surface"};
        }
        return vk::UniqueSurfaceKHR{ret, instance};
    }
    throw std::runtime_error("Invalid Window Handle!");
}

VulkanSwapchain VulkanBackend::CreateSwapchain(const WindowHandle* windowHandle, const vk::Device device, const GPU& gpu, 
    const vk::SurfaceKHR surface, const DispatchLoaderDynamic& loader)
{
    int width;
    int height;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_WindowHandle->nativeWindowHandle), &width, &height);
    return VulkanSwapchain(device, gpu, surface, loader, glm::ivec2{width, height});
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

VulkanBackend::VulkanDebugMessenger VulkanBackend::CreateMessenger(const vk::Instance instance, const DispatchLoaderDynamic& loader) {
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


    return instance.createDebugUtilsMessengerEXTUnique(
            createInfo,
            nullptr,
            loader
    );
}