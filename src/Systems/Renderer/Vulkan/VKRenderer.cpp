#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#include <GLFW/glfw3.h>
#include "ZeusEngineCore/core/Application.h"
#include <vulkan/vk_enum_string_helper.h>
#include "VKImages.h"
#include "VKInit.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include "VKPipelines.h"
#include "ZeusEngineCore/engine/CameraSystem.h"
#include "ZeusEngineCore/engine/Scene.h"
#include "ZeusEngineCore/engine/rendering/VKUtils.h"
#include "ZeusEngineCore/engine/Components.h"
#define IMGUI_IMG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

using namespace ZEN;

uint32_t TextureAllocator::allocate() {
    if (!availableList.empty()) {
        uint32_t idx = availableList.back();
        availableList.pop_back();
        return idx;
    }
    spdlog::warn("Renderer: No available slot for texture!");
    return 0;
}

void TextureAllocator::free(uint32_t idx) {
    freeList.push_back(idx);
}

void TextureAllocator::init(uint32_t maxTextures) {
    availableList.resize(maxTextures);
    for (uint32_t i = 0; i < maxTextures; i++) {
        availableList[i] = maxTextures - 1 - i;
    }
}

void TextureAllocator::flush() {
    availableList.insert(availableList.end(), freeList.begin(), freeList.end());
    freeList.clear();
}

VKRenderer::VKRenderer() {
}
//static GPUMeshBuffers cube;
void VKRenderer::init(EngineContext* ctx) {
    m_Scene = ctx->scene;
    m_CameraSystem = ctx->cameraSystem;

    initVulkan();
    initSampler();
    initSwapChain();
    initCommands();
    initSyncStructures();
    initDescriptors();
    initPipelines();

    //checkerboard image
    uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
    uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
    std::array<uint32_t, 16 *16 > pixels; //for 16x16 checkerboard texture
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            pixels[y*16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
        }
    }
    m_ErrorCheckerboardImage = createImage(pixels.data(), VkExtent3D{16, 16, 1}, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT);
    m_TextureAllocator.allocate(); //reserve 0

    VkSamplerCreateInfo sampl = {.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

    sampl.magFilter = VK_FILTER_NEAREST;
    sampl.minFilter = VK_FILTER_NEAREST;

    vkCreateSampler(m_Device, &sampl, nullptr, &m_DefaultSamplerNearest);

    sampl.magFilter = VK_FILTER_LINEAR;
    sampl.minFilter = VK_FILTER_LINEAR;
    vkCreateSampler(m_Device, &sampl, nullptr, &m_DefaultSamplerLinear);

    m_DeletionQueue.pushFunction([=](){
        destroyImage(m_ErrorCheckerboardImage);
        vkDestroySampler(m_Device,m_DefaultSamplerNearest,nullptr);
        vkDestroySampler(m_Device,m_DefaultSamplerLinear,nullptr);
    });

    m_Initialized = true;
}

static uint32_t swapChainImageIndex{};
void VKRenderer::beginFrame() {
    VK_CHECK(vkWaitForFences(m_Device, 1, &getCurrentFrame().m_Fence, true, 1000000000));
    getCurrentFrame().m_DeletionQueue.flush();
    getCurrentFrame().m_FrameDescriptors.clearPools(m_Device);
    VK_CHECK(vkResetFences(m_Device, 1, &getCurrentFrame().m_Fence));

    auto result = vkAcquireNextImageKHR(m_Device, m_SwapChain, 1000000000, getCurrentFrame().m_SwapChainSemaphore,
        nullptr, &swapChainImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            //recreate swapchain
        m_SwapChainRecreated = true;
        return;
        }

    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    VkCommandBufferBeginInfo cmdBeginInfo = VKInit::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_DrawExtent.width = m_DrawImage.imageExtent.width;
    m_DrawExtent.height = m_DrawImage.imageExtent.height;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //make SwapChain writable
    VKImages::transitionImage(cmd, m_DrawImage.image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}



void VKRenderer::draw() {
    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;

    drawBackground(cmd);
    VKImages::transitionImage(cmd, m_DrawImage.image,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
     VKImages::transitionImage(cmd, m_DepthImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    drawGeometry(cmd);

}
void VKRenderer::endFrame() {
    VkCommandBuffer cmd = getCurrentFrame().m_MainCommandBuffer;

    //we render to an image used by imgui, dont copy over to swapchain in this case

    /*--------------------------------------------------------NON-IMGUI--------------------------------------------*/
#ifndef IMGUI_IMG
    VKImages::transitionImage(cmd, m_DrawImage.image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VKImages::copyImageToImage(cmd, m_DrawImage.image, m_SwapChainImages[swapChainImageIndex],
        m_DrawExtent, m_SwapChainExtent);
    VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    /*--------------------------------------------------------NON-IMGUI--------------------------------------------*/

    /*--------------------------------------------------------IMGUI------------------------------------------------*/
#else
    VKImages::transitionImage(cmd, m_DrawImage.image,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
    VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    drawImgui(cmd, m_SwapChainImageViews[swapChainImageIndex]);
    VKImages::transitionImage(cmd, m_SwapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    /*--------------------------------------------------------IMGUI------------------------------------------------*/
#endif
    //make swapchain presentable
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

    auto result = vkQueuePresentKHR(m_GraphicsQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        //recreate swapchain
        m_SwapChainRecreated = true;
    }

    m_FrameNumber++;

    if (m_SwapChainRecreated) {
        recreateSwapChain();
    }
}

void VKRenderer::drawBackground(VkCommandBuffer cmd) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientPipelineLayout,
        0, 1, &m_DrawImageDescriptors, 0, nullptr);
    float dt = Application::get().getWindow()->getDeltaTime();
    static float totalTime = 0;
    totalTime += dt;
    vkCmdPushConstants(cmd, m_GradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float), &totalTime);

    vkCmdDispatch(cmd, std::ceil(m_DrawExtent.width / 16.0),
        std::ceil(m_DrawExtent.height / 16.0), 1);
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
        for (auto &buf: m_MeshMap | std::views::values) {
            destroyBuffer(buf.indexBuffer);
            destroyBuffer(buf.vertexBuffer);
        }
        for (auto &tex : m_TextureMap | std::views::values) {
            destroyImage(tex.image);
            //vkDestroySampler(m_Device, tex.sampler, nullptr);
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
    features12.descriptorBindingPartiallyBound = true;
    features12.descriptorBindingUpdateUnusedWhilePending = true;
    features12.descriptorBindingSampledImageUpdateAfterBind = true;
    features12.runtimeDescriptorArray = true;

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
    m_DeletionQueue.pushFunction([=]() {
        vmaDestroyAllocator(m_Allocator);
    });
    spdlog::debug("Renderer: Initialized State");
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
    drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageCreateInfo rImgInfo = VKInit::imageCreateInfo(m_DrawImage.imageFormat, drawImageUsages, drawImageExtent);

    VmaAllocationCreateInfo rImgAllocationCreateInfo {};
    rImgAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rImgAllocationCreateInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(m_Allocator, &rImgInfo, &rImgAllocationCreateInfo, &m_DrawImage.image, &m_DrawImage.allocation, nullptr);

    VkImageViewCreateInfo rViewInfo = VKInit::imageViewCreateInfo(m_DrawImage.image, m_DrawImage.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(m_Device, &rViewInfo, nullptr, &m_DrawImage.imageView));

    m_DepthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
    m_DepthImage.imageExtent = drawImageExtent;
    VkImageUsageFlags depthImageUsages{};
    depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkImageCreateInfo depthImageInfo = VKInit::imageCreateInfo(m_DepthImage.imageFormat, depthImageUsages, drawImageExtent);
    vmaCreateImage(m_Allocator, &depthImageInfo, &rImgAllocationCreateInfo, &m_DepthImage.image,
        &m_DepthImage.allocation, nullptr);

    VkImageViewCreateInfo depthViewInfo = VKInit::imageViewCreateInfo(m_DepthImage.image,
        m_DepthImage.imageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    VK_CHECK(vkCreateImageView(m_Device, &depthViewInfo, nullptr, &m_DepthImage.imageView));

    m_DeletionQueue.pushFunction([=]() {
        vkDestroyImageView(m_Device, m_DrawImage.imageView, nullptr);
        vmaDestroyImage(m_Allocator, m_DrawImage.image, m_DrawImage.allocation);

        vkDestroyImageView(m_Device, m_DepthImage.imageView, nullptr);
        vmaDestroyImage(m_Allocator, m_DepthImage.image, m_DepthImage.allocation);
    });
    spdlog::debug("Renderer: Initialized Swapchain");
}

void VKRenderer::initCommands() {
    VkCommandPoolCreateInfo commandPoolCreateInfo = VKInit::commandPoolCreateInfo(m_GraphicsQueueFamily,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (size_t i{}; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_Frames[i].m_CommandPool));

        VkCommandBufferAllocateInfo cmdAllocInfo = VKInit::commandBufferAllocateInfo(m_Frames[i].m_CommandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_Frames[i].m_MainCommandBuffer));
    }

    //immediate submit
    VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_ImmediateCommandPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = VKInit::commandBufferAllocateInfo(m_ImmediateCommandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_ImmediateCommandBuffer));

    m_DeletionQueue.pushFunction([=]() {
    vkDestroyCommandPool(m_Device, m_ImmediateCommandPool, nullptr);
    });
    spdlog::debug("Renderer: Initialized Commands");
}

void VKRenderer::initSyncStructures() {
    VkFenceCreateInfo fenceCreateInfo = VKInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = VKInit::semaphoreCreateInfo();

    for (int i{}; i < FRAME_OVERLAP; ++i) {
        VK_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_Frames[i].m_Fence));
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].m_SwapChainSemaphore));
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].m_RenderSemaphore));
    }
    VK_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_ImmediateFence));
    m_DeletionQueue.pushFunction([=]() {
        vkDestroyFence(m_Device, m_ImmediateFence, nullptr);
    });
    spdlog::debug("Renderer: Initialized Sync Structures");
}

void VKRenderer::initDescriptors() {
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
    };
    m_GlobalDescriptorAllocator.initPool(m_Device, 10, sizes);
    {
        DescriptorLayoutBuilder builder;
        builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        m_DrawImageDescriptorLayout = builder.build(m_Device, VK_SHADER_STAGE_COMPUTE_BIT);
    }
    {
        DescriptorLayoutBuilder builder;
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

        m_TextureAllocator.init(props.limits.maxDescriptorSetSampledImages);

        builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, props.limits.maxDescriptorSetSampledImages,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);

        builder.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        m_MainDescriptorLayout = builder.build(m_Device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);

        m_DeletionQueue.pushFunction([=]() {
            vkDestroyDescriptorSetLayout(m_Device, m_MainDescriptorLayout, nullptr);
        });
    }
    m_DrawImageDescriptors = m_GlobalDescriptorAllocator.allocate(m_Device, m_DrawImageDescriptorLayout);

    {
        DescriptorWriter writer;
        writer.writeImage(0, m_DrawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        writer.updateSet(m_Device, m_DrawImageDescriptors);
    }

    m_DeletionQueue.pushFunction([=]() {
        m_GlobalDescriptorAllocator.destroyPool(m_Device);
        vkDestroyDescriptorSetLayout(m_Device, m_DrawImageDescriptorLayout, nullptr);
    });

    for (int i{}; i < FRAME_OVERLAP; ++i) {
        std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frameSizes {
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
        };
        m_Frames[i].m_FrameDescriptors = DescriptorAllocatorGrowable{};
        m_Frames[i].m_FrameDescriptors.init(m_Device, 1000, frameSizes);
        m_DeletionQueue.pushFunction([=]() {
            m_Frames[i].m_FrameDescriptors.destroyPools(m_Device);
        });
    }
    spdlog::debug("Renderer: Initialized Descriptors");


}

void VKRenderer::initPipelines() {
    initBackgroundPipeline();
    initMeshPipeline();
}

void VKRenderer::initBackgroundPipeline() {
    VkPushConstantRange push{};
    push.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push.offset = 0;
    push.size = sizeof(float);

    VkPipelineLayoutCreateInfo computeLayout = VKInit::pipelineLayoutCreateInfo();
    computeLayout.pSetLayouts = &m_DrawImageDescriptorLayout;
    computeLayout.setLayoutCount = 1;
    computeLayout.pushConstantRangeCount = 1;
    computeLayout.pPushConstantRanges = &push;

    VK_CHECK(vkCreatePipelineLayout(m_Device, &computeLayout, nullptr, &m_GradientPipelineLayout));

    VkShaderModule computeDrawShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        "/shaders/vulkan-shaders/gradient.comp.spv", m_Device, &computeDrawShader)) {
        std::cout << "Failed to load compute draw shader" << std::endl;
    }
    VkPipelineShaderStageCreateInfo stageInfo = VKInit::pipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_COMPUTE_BIT, computeDrawShader);

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_GradientPipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    VK_CHECK(vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
        &m_GradientPipeline));

    vkDestroyShaderModule(m_Device, computeDrawShader, nullptr);
    m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipelineLayout(m_Device, m_GradientPipelineLayout, nullptr);
        vkDestroyPipeline(m_Device, m_GradientPipeline, nullptr);
    });
    spdlog::debug("Renderer: Initialized Background Compute Pipeline");
}

void VKRenderer::initSampler() {
    VkSamplerCreateInfo samplerInfo{};
    //todo make these editable
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler));
    m_DeletionQueue.pushFunction([=]() {
        vkDestroySampler(m_Device, m_Sampler, nullptr);
    });
}

void VKRenderer::initMeshPipeline() {
    VkShaderModule triangleFragShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        "/shaders/vulkan-shaders/testTriangle.frag.spv", m_Device, &triangleFragShader)) {
        std::cout << "Failed to load triangle frag shader" << std::endl;
    }
    VkShaderModule triangleVertShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        "/shaders/vulkan-shaders/testTriangle.vert.spv", m_Device, &triangleVertShader)) {
        std::cout << "Failed to load triangle vert shader" << std::endl;
    }
    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo layoutInfo = VKInit::pipelineLayoutCreateInfo();
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;

    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &m_MainDescriptorLayout;

    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &bufferRange;
    VK_CHECK(vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_MeshPipelineLayout));

    VKPipelineBuilder builder;
    builder.pipelineLayout = m_MeshPipelineLayout;
    builder.setShaders(triangleVertShader, triangleFragShader);
    builder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    builder.setCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setMultiSamplingNone();
    //builder.enableBlendingAdditive();
    builder.disableBlending();
    //builder.disableDepthTest();
    builder.setColorAttachmentFormat(m_DrawImage.imageFormat);
    builder.enableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    builder.setDepthFormat(m_DepthImage.imageFormat);

    m_MeshPipeline = builder.buildPipeline(m_Device);

    vkDestroyShaderModule(m_Device, triangleFragShader, nullptr);
    vkDestroyShaderModule(m_Device, triangleVertShader, nullptr);

    m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipelineLayout(m_Device, m_MeshPipelineLayout, nullptr);
        vkDestroyPipeline(m_Device, m_MeshPipeline, nullptr);
    });
    spdlog::debug("Renderer: Initialized Mesh Pipeline");
}

void VKRenderer::drawImgui(VkCommandBuffer cmd, VkImageView targetImageView) {
    VkRenderingAttachmentInfo colorAttachment = VKInit::attachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VKInit::renderingInfo(m_SwapChainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

void VKRenderer::drawGeometry(VkCommandBuffer cmd) {
    //create new uniform buff for scene data
    AllocatedBuffer gpuSceneDataBuffer = createBuffer(sizeof(GPUSceneData),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    getCurrentFrame().m_DeletionQueue.pushFunction([=]() {
        destroyBuffer(gpuSceneDataBuffer);
    });

    //write to buffer
    auto* sceneUniformData = (GPUSceneData*)gpuSceneDataBuffer.allocation->GetMappedData();
    m_SceneData = {};
    m_SceneData.viewProj = m_CameraSystem->getVP();
    m_SceneData.ambientColor = {0.3f, 0.3f, 0.3f, 0.3f};
    m_SceneData.sunlightColor = {1.0f, 1.0f, 1.0f, 1.0f};
    auto lightDir = m_Scene->getLightDir();
    glm::vec4 light = glm::vec4(lightDir.x, lightDir.y, lightDir.z, 1);
    m_SceneData.sunlightDirection = light;
    *sceneUniformData = m_SceneData;

    //create descriptor set that binds this buffer and updates it
    VkDescriptorSet globalDescriptor = getCurrentFrame().m_FrameDescriptors.
    allocate(m_Device, m_MainDescriptorLayout);

    DescriptorWriter writer;
    writer.writeBuffer(1, gpuSceneDataBuffer.buffer,
        sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.updateSet(m_Device, globalDescriptor);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_MeshPipelineLayout,0, 1, &globalDescriptor,0,nullptr);

    VkRenderingAttachmentInfo colorAttInfo = VKInit::attachmentInfo(m_DrawImage.imageView
        , nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttInfo = VKInit::depthAttachmentInfo(m_DepthImage.imageView
        , VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VKInit::renderingInfo(m_DrawExtent,
        &colorAttInfo, &depthAttInfo);

    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshPipeline);

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = m_DrawExtent.width;
    viewport.height = m_DrawExtent.height;
    viewport.minDepth = 1.0f;
    viewport.maxDepth = 0.0f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = m_DrawExtent.width;
    scissor.extent.height = m_DrawExtent.height;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    GPUDrawPushConstants pushConstants;

    for (auto& [id, tex] : m_TextureMap) {
        writer.writeImage(0, tex.image.imageView, tex.sampler,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, tex.index);
    }
    writer.writeImage(0, m_ErrorCheckerboardImage.imageView, m_DefaultSamplerNearest,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);

    writer.updateSet(m_Device, globalDescriptor);

    //todo probably move this logic out of here and have some sort of scene renderer submit render info
    //involving material etc
    for (auto entity : m_Scene->getEntities<TransformComp, MeshComp>()) {
        auto meshID = entity.getComponent<MeshComp>().handle.id();
        auto buf = m_MeshMap[meshID];

        auto model = entity.getComponent<TransformComp>().worldMatrix;
        pushConstants.worldMatrix = model;
        pushConstants.vertexBuffer = buf.vertexBufferAddress;

        auto tex = GPUTexture{.image = m_ErrorCheckerboardImage, .sampler = m_DefaultSamplerNearest, .index = 0};
        auto mat = entity.tryGetComponent<MaterialComp>();
        if (mat) {
            auto texID = mat->handle->texture;
            if (m_TextureMap.contains(texID)) {
                tex = m_TextureMap[texID];
            }
        }
        pushConstants.albedoIndex = tex.index;

        vkCmdPushConstants(cmd, m_MeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
        vkCmdBindIndexBuffer(cmd, buf.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, buf.indexCount, 1, 0, 0, 0);
    }

    vkCmdEndRendering(cmd);
}

void VKRenderer::immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function) {
    VK_CHECK(vkResetFences(m_Device, 1, &m_ImmediateFence));
    VK_CHECK(vkResetCommandPool(m_Device, m_ImmediateCommandPool, 0));

    VkCommandBuffer cmd = m_ImmediateCommandBuffer;

    VkCommandBufferBeginInfo beginInfo = VKInit::cmdBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdInfo = VKInit::cmdBufferSubmitInfo(cmd);
    VkSubmitInfo2 submitInfo = VKInit::submitInfo(&cmdInfo, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submitInfo, m_ImmediateFence));
    VK_CHECK(vkWaitForFences(m_Device, 1, &m_ImmediateFence, VK_TRUE, UINT64_MAX));
}

AllocatedImage VKRenderer::createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
    AllocatedImage newImage;
    newImage.imageFormat = format;
    newImage.imageExtent = size;

    VkImageCreateInfo imgInfo = VKInit::imageCreateInfo(format, usage, size);
    if (mipmapped) {
        imgInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
    }

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vmaCreateImage(m_Allocator, &imgInfo, &allocInfo, &newImage.image, &newImage.allocation, nullptr));

    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format == VK_FORMAT_D32_SFLOAT) {
        aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    VkImageViewCreateInfo viewInfo = VKInit::imageViewCreateInfo(newImage.image, format, aspectFlag);
    viewInfo.subresourceRange.levelCount = imgInfo.mipLevels;

    VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &newImage.imageView));

    spdlog::debug("Renderer: Created Image");

    return newImage;
}

AllocatedImage VKRenderer::createImage(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
    bool mipmapped) {
    size_t dataSize = size.depth * size.width * size.height * 4; //todo check if can use sizeof here
    AllocatedBuffer uploadBuffer = createBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    memcpy(uploadBuffer.allocationInfo.pMappedData, data, dataSize);

    AllocatedImage newImage = createImage(size, format,
        usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

    immediateSubmit([&](VkCommandBuffer cmd) {
        VKImages::transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = size;

        vkCmdCopyBufferToImage(cmd, uploadBuffer.buffer, newImage.image,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
            &copyRegion);

        VKImages::transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        spdlog::debug("Renderer: Uploaded Image");
    });

    destroyBuffer(uploadBuffer);
    return newImage;
}

void VKRenderer::destroyImage(const AllocatedImage &img) {
    vkDestroyImageView(m_Device, img.imageView, nullptr);
    vmaDestroyImage(m_Allocator, img.image, img.allocation);
    spdlog::debug("Renderer: Deleted Image");
}

ImGui_ImplVulkan_InitInfo VKRenderer::initImgui() {
    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &imguiPool));

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_Instance;
    initInfo.PhysicalDevice = m_PhysicalDevice;
    initInfo.Device = m_Device;
    initInfo.Queue = m_GraphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;

    //dynamic rendering parameters stuff
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_SwapChainImageFormat;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
    m_ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_DrawImage.imageView,
        VK_IMAGE_LAYOUT_GENERAL
    );

    m_DeletionQueue.pushFunction([=]() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(m_Device, imguiPool, nullptr);

    });
    spdlog::debug("Renderer: ImGUI Initialized");

    return initInfo;
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

    spdlog::debug("Renderer: Swapchain Created");

}

void VKRenderer::recreateSwapChain() {
    int width, height;
    glfwGetFramebufferSize(Application::get().getWindow()->getNativeWindow(), &width, &height);
    vkDeviceWaitIdle(m_Device);
    destroySwapChain();
    m_DrawExtent.width = width;
    m_DrawExtent.height = height;
    createSwapChain(width, height);
    m_SwapChainRecreated = false;
    spdlog::debug("Renderer: Swapchain Recreated");
}

void VKRenderer::destroySwapChain() {
    vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);

    for (size_t i{}; i < m_SwapChainImageViews.size(); ++i) {
        vkDestroyImageView(m_Device, m_SwapChainImageViews[i], nullptr);
    }
    spdlog::debug("Renderer: Swapchain Destroyed");
}

AllocatedBuffer VKRenderer::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;
    bufferInfo.pNext = nullptr;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    AllocatedBuffer buffer;

    VK_CHECK(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &buffer.buffer,
        &buffer.allocation, &buffer.allocationInfo));

    //spdlog::debug("Renderer: Buffer Created");

    return buffer;
}

void VKRenderer::destroyBuffer(const AllocatedBuffer &buffer) {
    vmaDestroyBuffer(m_Allocator, buffer.buffer, buffer.allocation);
    //spdlog::debug("Renderer: Buffer Destroyed");
}

GPUMeshBuffers VKRenderer::uploadMesh(AssetID id, const MeshData &mesh) {

    if (m_MeshMap.find(id) != m_MeshMap.end()) {
        spdlog::warn("AssetID already exists in GPU, overwriting..");
        deleteMesh(id);
    }

    const size_t vertexBufferSize = mesh.vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = mesh.indices.size() * sizeof(uint32_t);

    GPUMeshBuffers newSurface;
    newSurface.vertexBuffer = createBuffer(vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAddressInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = newSurface.vertexBuffer.buffer,
    };
    newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_Device, &deviceAddressInfo);

    newSurface.indexBuffer = createBuffer(indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = createBuffer(vertexBufferSize + indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = staging.allocation->GetMappedData();

    memcpy(data, mesh.vertices.data(), vertexBufferSize);
    memcpy((char*)data + vertexBufferSize, mesh.indices.data(), indexBufferSize);

    immediateSubmit([&](VkCommandBuffer cmd) {
        VkBufferCopy vertCopy{0};
        vertCopy.dstOffset = 0;
        vertCopy.srcOffset = 0;
        vertCopy.size = vertexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer,
            newSurface.vertexBuffer.buffer, 1, &vertCopy);

        VkBufferCopy indexCopy{0};
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer,
            newSurface.indexBuffer.buffer, 1, &indexCopy);
    });
    newSurface.indexCount = mesh.indices.size();

    destroyBuffer(staging);
    m_MeshMap[id] = newSurface;

    spdlog::debug("Renderer: Created GPU Mesh ID: {} of index count: {}", (uint64_t)id, newSurface.indexCount);
    return newSurface;
}

GPUTexture VKRenderer::uploadTexture(AssetID id, const TextureData &texture) {
    int texWidth, texHeight, texChannels;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels;
    bool allocatedByUs = true;

    //---------------------ASSIMP TEXTURE--------------------
    if (texture.aiTex && texture.aiTex->mHeight == 0) {
        pixels = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture.aiTex->pcData),texture.aiTex->mWidth,
    &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    } else if (texture.aiTex) {
        texWidth = texture.aiTex->mWidth;
        texHeight = texture.aiTex->mHeight;
        texChannels = 4;
        pixels = reinterpret_cast<unsigned char*>(texture.aiTex->pcData);
        allocatedByUs = false;
    //--------------------------------------------------------
    //---------------------PATH TEXTURE-----------------------
    } else {
        pixels = stbi_load(texture.path.data(), &texWidth,
                                    &texHeight, &texChannels, STBI_rgb_alpha);
    }
    //--------------------------------------------------------

    if (!pixels && allocatedByUs) {
        std::cout<<"Invalid Image! Assigning default texture.."<<"\n";
        return {};
    }

    AllocatedImage newTexture = createImage((void*)pixels, VkExtent3D{(unsigned int)texWidth, (unsigned int)texHeight, 1}, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT);

    uint32_t index = m_TextureAllocator.allocate();

    GPUTexture ret = {
        .image = newTexture,
        .sampler = m_DefaultSamplerNearest,
        .index = index,
    };

    stbi_image_free(pixels);

    m_TextureMap[id] = ret;

    spdlog::debug("Renderer: Created Texture ID: {}", (uint64_t)id);

    return ret;
}

void VKRenderer::deleteMesh(AssetID id) {
    if (m_MeshMap.find(id) != m_MeshMap.end()) {
        auto meshBuf = m_MeshMap[id];
        getCurrentFrame().m_DeletionQueue.pushFunction([=]() {
            destroyBuffer(meshBuf.indexBuffer);
            destroyBuffer(meshBuf.vertexBuffer);
        });
        m_MeshMap.erase(id);
        spdlog::debug("Deleted GPU Mesh ID: {}", (uint64_t)id);
        return;
    }
    spdlog::error("Attempt to delete non-existing GPU Mesh! ID: {}", (uint64_t)id);
}

void VKRenderer::removeTexture(AssetID id) {
    if (m_TextureMap.find(id) != m_TextureMap.end()) {
        auto tex = m_TextureMap[id];
        getCurrentFrame().m_DeletionQueue.pushFunction([=]() {
            m_TextureAllocator.free(tex.index);
            destroyImage(tex.image);
            //vkDestroySampler(m_Device, tex.sampler, nullptr);
        });
        m_MeshMap.erase(id);
        spdlog::debug("Deleted Texture ID: {}", (uint64_t)id);
        return;
    }
    spdlog::error("Attempt to delete non-existing Texture! ID: {}", (uint64_t)id);
}
