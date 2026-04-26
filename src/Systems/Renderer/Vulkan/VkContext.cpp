#include "ZeusEngineCore/core/Application.h"
#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>

#include "VKInit.h"
#include "VKPipelines.h"
#include "ZeusEngineCore/engine/rendering/VKUtils.h"

using namespace ZEN;

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

    DescriptorWriter writer;
    writer.writeImage(0, m_ErrorCheckerboardImage.imageView, m_DefaultSamplerNearest,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);

    writer.updateSet(m_Device, m_TextureDescriptorSet);

    m_DeletionQueue.pushFunction([=](){
        destroyImage(m_ErrorCheckerboardImage);
        vkDestroySampler(m_Device,m_DefaultSamplerNearest,nullptr);
        vkDestroySampler(m_Device,m_DefaultSamplerLinear,nullptr);
    });

    m_Initialized = true;
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
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
    };
    m_GlobalDescriptorAllocator.initPool(m_Device, 10, sizes);

    std::vector<DescriptorAllocator::PoolSizeRatio> textureSizes {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    };
    m_TextureDescriptorAllocator.initPool(m_Device, 1000, textureSizes);
    {
        DescriptorLayoutBuilder builder;
        builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        m_DrawImageDescriptorLayout = builder.build(m_Device, VK_SHADER_STAGE_COMPUTE_BIT);
    }
    {
        DescriptorLayoutBuilder builder;
        builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        m_MainDescriptorLayout = builder.build(m_Device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);

        m_DeletionQueue.pushFunction([=]() {
            vkDestroyDescriptorSetLayout(m_Device, m_MainDescriptorLayout, nullptr);
        });
    }
    {
        DescriptorLayoutBuilder builder;
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

        m_TextureAllocator.init(props.limits.maxDescriptorSetSampledImages);

        builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, props.limits.maxDescriptorSetSampledImages,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
        m_TextureDescriptorSetLayout = builder.build(m_Device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);

        m_DeletionQueue.pushFunction([=]() {
            vkDestroyDescriptorSetLayout(m_Device, m_TextureDescriptorSetLayout, nullptr);
        });
    }
    m_DrawImageDescriptors = m_GlobalDescriptorAllocator.allocate(m_Device, m_DrawImageDescriptorLayout);
    m_TextureDescriptorSet = m_TextureDescriptorAllocator.allocate(m_Device, m_TextureDescriptorSetLayout);
    {
        DescriptorWriter writer;
        writer.writeImage(0, m_DrawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        writer.updateSet(m_Device, m_DrawImageDescriptors);
    }

    m_DeletionQueue.pushFunction([=]() {
        m_TextureDescriptorAllocator.destroyPool(m_Device);
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

    std::array<VkDescriptorSetLayout, 2> setLayouts = {
        m_MainDescriptorLayout,
        m_TextureDescriptorSetLayout
    };

    layoutInfo.pSetLayouts = setLayouts.data();
    layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());

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

VKRenderer::~VKRenderer() {
    cleanup();
}