#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#include <GLFW/glfw3.h>
#include "ZeusEngineCore/core/Application.h"
using namespace ZEN;
VKRenderer::VKRenderer() {
    init();
}

void VKRenderer::init() {
    initVulkan();
    initSwapChain();
    initCommands();
    initSyncStructures();

    m_Initialized = true;
}

void VKRenderer::cleanup() {
    if (m_Initialized) {
        destroySwapChain();

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyDevice(m_Device, nullptr);

        vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
        vkDestroyInstance(m_Instance, nullptr);
    }
}

VKRenderer::~VKRenderer() {
    cleanup();
}

void VKRenderer::initVulkan() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    vkb::InstanceBuilder builder;

    builder.set_app_name("Zeus Engine");
#ifdef NDEBUG
    builder.request_validation_layers(false);
#else
    builder.request_validation_layers(true);
#endif
    builder.use_default_debug_messenger();
    builder.enable_extensions(glfwExtensionCount, glfwExtensions);
    builder.require_api_version(1, 3, 0);

    auto instRet = builder.build();

    vkb::Instance vkbInst = instRet.value();

    m_Instance = vkbInst.instance;
    m_DebugMessenger = vkbInst.debug_messenger;

    if (glfwCreateWindowSurface(m_Instance, Application::get().getWindow()->getNativeWindow(), nullptr,
        &m_Surface) != VK_SUCCESS) {
        m_Initialized = false;
        throw std::runtime_error("Failed to create window surface!");
    }

    VkPhysicalDeviceVulkan13Features features13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector {vkbInst};
    selector.set_minimum_version(1, 3);
    selector.set_required_features_13(features13);
    selector.set_required_features_12(features12);
    selector.set_surface(m_Surface);

    vkb::PhysicalDevice physicalDevice = selector.select().value();

    vkb::DeviceBuilder deviceBuilder {physicalDevice};
    vkb::Device vkbDevice = deviceBuilder.build().value();

    m_Device = vkbDevice.device;
    m_PhysicalDevice = physicalDevice.physical_device;
}

void VKRenderer::initSwapChain() {
    int width, height;
    glfwGetFramebufferSize(Application::get().getWindow()->getNativeWindow(), &width, &height);
    createSwapChain(width, height);
}

void VKRenderer::initCommands() {

}

void VKRenderer::initSyncStructures() {

}

void VKRenderer::createSwapChain(uint32_t width, uint32_t height) {
    vkb::SwapchainBuilder swapChainBuilder{m_PhysicalDevice, m_Device, m_Surface};

    m_SwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    swapChainBuilder.set_desired_format(VkSurfaceFormatKHR{.format = m_SwapChainImageFormat});
    swapChainBuilder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    swapChainBuilder.set_desired_extent(width, height);
    swapChainBuilder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkb::Swapchain vkbSwapChain = swapChainBuilder.build().value();

    m_SwapChainExtent = vkbSwapChain.extent;

    m_SwapChain = vkbSwapChain.swapchain;
    m_SwapChainImages = vkbSwapChain.get_images().value();
    m_SwapChainImageViews = vkbSwapChain.get_image_views().value();

}

void VKRenderer::destroySwapChain() {
    vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);

    for (int i = 0; i < m_SwapChainImageViews.size(); i++) {
        vkDestroyImageView(m_Device, m_SwapChainImageViews[i], nullptr);
    }
}
