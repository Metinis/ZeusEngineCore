#include "VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(vk::Device device, GPU const& gpu, vk::SurfaceKHR surface,
    const DispatchLoaderDynamic& dynamicLoader, glm::ivec2 size) :
	m_Device(device),
	m_GPU(gpu),
    m_DynamicLoader(dynamicLoader)
{
    const vk::SurfaceFormatKHR surfaceFormat = getSurfaceFormat(m_GPU.device.getSurfaceFormatsKHR(surface));
    /*m_CreateInfo.setSurface(surface)
            .setImageFormat(surfaceFormat.format)
            .setImageColorSpace(surfaceFormat.colorSpace)
            .setImageArrayLayers(1)
                    // Swapchain images will be used as color attachments (render targets).
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                    // eFifo is guaranteed to be supported.
            .setPresentMode(vk::PresentModeKHR::eFifo);*/
    m_CreateInfo.surface = surface;
    m_CreateInfo.imageFormat = surfaceFormat.format;
    m_CreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    m_CreateInfo.imageArrayLayers = 1;
    m_CreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    m_CreateInfo.presentMode = vk::PresentModeKHR::eFifo;
    m_CreateInfo.flags = {};
    m_CreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    m_CreateInfo.clipped = VK_TRUE;

    VkBool32 surfaceSupported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(
            gpu.device,
            gpu.queue_family,
            surface,
            &surfaceSupported
    );

    if (!surfaceSupported) {
        throw std::runtime_error("Selected physical device does not support presenting to this surface");
    }


    if (!recreate(size)) {
        throw std::runtime_error{ "Failed to create Vulkan Swapchain" };
    }
}

bool VulkanSwapchain::recreate(glm::ivec2 size)
{
    if (size.x <= 0 || size.y <= 0) { return false; }

    auto const capabilities = m_GPU.device.getSurfaceCapabilitiesKHR(m_CreateInfo.surface);
    m_CreateInfo.imageExtent = getImageExtent(capabilities, size);
    m_CreateInfo.minImageCount = getImageCount(capabilities);
    m_CreateInfo.oldSwapchain = m_Swapchain ? *m_Swapchain : vk::SwapchainKHR{};
    m_CreateInfo.preTransform = capabilities.currentTransform;

    //m_CreateInfo.queueFamilyIndexCount = 1;
    //m_CreateInfo.pQueueFamilyIndices = &m_GPU.queue_family;

    //since we only use one queue
    m_CreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    m_CreateInfo.queueFamilyIndexCount = 0;
    m_CreateInfo.pQueueFamilyIndices = nullptr;

    assert(m_CreateInfo.imageExtent.width > 0 && m_CreateInfo.imageExtent.height > 0 &&  m_CreateInfo.minImageCount >= minImages_v);

    m_Device.waitIdle();
    m_Swapchain = m_Device.createSwapchainKHRUnique(m_CreateInfo, nullptr, m_DynamicLoader);

    populateImages();
    createImageViews();

    size = getSize();
    //std::println("[lvk] Swapchain [{}x{}]", size.x, size.y);

    return true;
}

void VulkanSwapchain::populateImages()
{
    //avoiding a new vector every call
    std::uint32_t imageCount = 0;
    vk::Result result = m_Device.getSwapchainImagesKHR(*m_Swapchain, &imageCount, nullptr);
    requireSuccess(result, "Failed to get Swapchain Images");

    m_Images.resize(imageCount);
    result = m_Device.getSwapchainImagesKHR(*m_Swapchain, &imageCount, m_Images.data());
    requireSuccess(result, "Failed to get Swapchain Images");
}

void VulkanSwapchain::createImageViews()
{
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = 1;
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.subresourceRange = subresourceRange;
    imageViewCreateInfo.format = m_CreateInfo.imageFormat;

    m_ImageViews.clear();
    m_ImageViews.reserve(m_Images.size());
    for (auto const image : m_Images) {
        imageViewCreateInfo.image = image;
        m_ImageViews.push_back(m_Device.createImageViewUnique(imageViewCreateInfo));
    }

}

vk::SurfaceFormatKHR VulkanSwapchain::getSurfaceFormat(std::span<const vk::SurfaceFormatKHR> supported)
{
    for (auto const desired : srgbFormats_v) {
        auto const isMatch = [desired](vk::SurfaceFormatKHR const& in) {
            return in.format == desired &&
                in.colorSpace ==
                vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear;
            };
        auto const it = std::ranges::find_if(supported, isMatch);
        if (it == supported.end()) { continue; }
        return *it;
    }
    return supported.front();
}

vk::Extent2D VulkanSwapchain::getImageExtent(vk::SurfaceCapabilitiesKHR const& capabilities, glm::uvec2 const size)
{
    constexpr auto limitless_v = 0xffffffff;
    if (capabilities.currentExtent.width < limitless_v &&
        capabilities.currentExtent.height < limitless_v) {
        return capabilities.currentExtent;
    }
    auto const x = std::clamp(size.x, capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width);
    auto const y = std::clamp(size.y, capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);
    return vk::Extent2D{ x, y };
}

std::uint32_t VulkanSwapchain::getImageCount(vk::SurfaceCapabilitiesKHR const& capabilities)
{
    if (capabilities.maxImageCount < capabilities.minImageCount) {
        return std::max(minImages_v, capabilities.minImageCount);
    }
    return std::clamp(minImages_v, capabilities.minImageCount,
        capabilities.maxImageCount);
}
