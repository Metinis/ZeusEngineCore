#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#include <GLFW/glfw3.h>
#include "ZeusEngineCore/core/Application.h"
#include <vulkan/vk_enum_string_helper.h>
#include "VKImages.h"
#include "VKInit.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

using namespace ZEN;

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            const std::string errMsg = "Detected Vulkan error: " + std::string(string_VkResult(err));                                                            \
            throw std::runtime_error(errMsg);\
        }                                                               \
    } while (0)

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

void VKRenderer::draw() {
    VK_CHECK(vkWaitForFences(m_Device, 1, &getCurrentFrame().m_Fence, true, 1000000000));
    getCurrentFrame().m_DeletionQueue.flush();
    VK_CHECK(vkResetFences(m_Device, 1, &getCurrentFrame().m_Fence));

    uint32_t swapChainImageIndex{};
    VK_CHECK(vkAcquireNextImageKHR(m_Device, m_SwapChain, 1000000000, getCurrentFrame().m_SwapChainSemaphore,
        nullptr, &swapChainImageIndex));

    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    VkCommandBufferBeginInfo cmdBeginInfo = VKInit::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_DrawExtent.width = m_DrawExtent.width;
    m_DrawExtent.height = m_DrawExtent.height;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //make SwapChain writable
    VKImages::transitionImage(cmd, m_DrawImage.image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    drawBackground(cmd);

    //make swapchain presentable
    VKImages::transitionImage(cmd, m_DrawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VKImages::copyImageToImage(cmd, m_DrawImage.image, m_SwapChainImages[swapChainImageIndex],
        m_DrawExtent, m_SwapChainExtent);

    VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdSubmitInfo = VKInit::cmdBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo waitInfo = VKInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        getCurrentFrame().m_SwapChainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = VKInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        getCurrentFrame().m_RenderSemaphore);

    VkSubmitInfo2 submit = VKInit::submitInfo(&cmdSubmitInfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submit, getCurrentFrame().m_Fence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &getCurrentFrame().m_RenderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapChainImageIndex;

    VK_CHECK(vkQueuePresentKHR(m_GraphicsQueue, &presentInfo));

    m_FrameNumber++;
}

void VKRenderer::drawBackground(VkCommandBuffer cmd) {
    VkClearColorValue clearColorValue;
    float flash = std::abs(std::sin(m_FrameNumber / 120.0f));
    clearColorValue = {{0.0f, 0.0f, flash, 1.0f}};
    VkImageSubresourceRange clearRange = VKInit::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    vkCmdClearColorImage(cmd, m_DrawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearColorValue, 1, &clearRange);
}

void VKRenderer::cleanup() {
    if (m_Initialized) {
        vkDeviceWaitIdle(m_Device);
        for (int i{}; i < FRAME_OVERLAP; ++i) {
            vkDestroyCommandPool(m_Device, m_Frames[i].m_CommandPool, nullptr);

            vkDestroyFence(m_Device, m_Frames[i].m_Fence, nullptr);
            vkDestroySemaphore(m_Device, m_Frames[i].m_RenderSemaphore, nullptr);
            vkDestroySemaphore(m_Device, m_Frames[i].m_SwapChainSemaphore, nullptr);
            m_Frames[i].m_DeletionQueue.flush();
        }
        m_DeletionQueue.flush();
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

    m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    //allocator
    VmaAllocatorCreateInfo allocatorCreateInfo {};
    allocatorCreateInfo.device = m_Device;
    allocatorCreateInfo.instance = m_Instance;
    allocatorCreateInfo.physicalDevice = m_PhysicalDevice;
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator);
    m_DeletionQueue.pushFunction([&]() {
        vmaDestroyAllocator(m_Allocator);
    });
}

void VKRenderer::initSwapChain() {
    int width, height;
    glfwGetFramebufferSize(Application::get().getWindow()->getNativeWindow(), &width, &height);
    createSwapChain(width, height);

    VkExtent3D drawImageExtent {
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .depth = 1
    };

    m_DrawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_DrawImage.imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo rImgInfo = VKInit::imageCreateInfo(m_DrawImage.imageFormat, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo rImgAllocationCreateInfo {};
    rImgAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rImgAllocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(m_Allocator, &rImgInfo, &rImgAllocationCreateInfo, &m_DrawImage.image, &m_DrawImage.allocation, nullptr);

    VkImageViewCreateInfo rViewInfo = VKInit::imageViewCreateInfo(m_DrawImage.image, m_DrawImage.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(m_Device, &rViewInfo, nullptr, &m_DrawImage.imageView));

    m_DeletionQueue.pushFunction([&]() {
        vkDestroyImageView(m_Device, m_DrawImage.imageView, nullptr);
        vmaDestroyImage(m_Allocator, m_DrawImage.image, nullptr);
    });
}

void VKRenderer::initCommands() {
    VkCommandPoolCreateInfo commandPoolCreateInfo = VKInit::commandPoolCreateInfo(m_GraphicsQueueFamily,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (size_t i{}; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_Frames[i].m_CommandPool));

        VkCommandBufferAllocateInfo cmdAllocInfo = VKInit::commandBufferAllocateInfo(m_Frames[i].m_CommandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_Frames[i].m_MainCommandBuffer));
    }
}

void VKRenderer::initSyncStructures() {
    VkFenceCreateInfo fenceCreateInfo = VKInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = VKInit::semaphoreCreateInfo();

    for (int i{}; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_Frames[i].m_Fence));
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].m_SwapChainSemaphore));
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].m_RenderSemaphore));
    }
}

void VKRenderer::createSwapChain(uint32_t width, uint32_t height) {
    vkb::SwapchainBuilder swapChainBuilder{m_PhysicalDevice, m_Device, m_Surface};

    m_SwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    swapChainBuilder.set_desired_format(VkSurfaceFormatKHR{.format = m_SwapChainImageFormat});
    swapChainBuilder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    swapChainBuilder.set_desired_extent(width, height);
    swapChainBuilder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    swapChainBuilder.set_desired_min_image_count(FRAME_OVERLAP);
    vkb::Swapchain vkbSwapChain = swapChainBuilder.build().value();

    m_SwapChainExtent = vkbSwapChain.extent;

    m_SwapChain = vkbSwapChain.swapchain;
    m_SwapChainImages = vkbSwapChain.get_images().value();
    m_SwapChainImageViews = vkbSwapChain.get_image_views().value();

}

void VKRenderer::destroySwapChain() {
    vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);

    for (size_t i{}; i < m_SwapChainImageViews.size(); ++i) {
        vkDestroyImageView(m_Device, m_SwapChainImageViews[i], nullptr);
    }
}
