
#include "Swapchain.h"
#include <vulkan/vulkan.hpp>

using namespace ZEN::VKAPI;

Swapchain::Swapchain(vk::Device device, GPU const& gpu, vk::SurfaceKHR surface, glm::ivec2 size) :
	m_Device(device),
	m_GPU(gpu)
{
    const vk::SurfaceFormatKHR surfaceFormat = GetSurfaceFormat(m_GPU.device.getSurfaceFormatsKHR(surface));
    m_CreateInfo.surface = surface;
    m_CreateInfo.imageFormat = surfaceFormat.format;
    m_CreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    m_CreateInfo.imageArrayLayers = 1;
    m_CreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    m_CreateInfo.presentMode = vk::PresentModeKHR::eFifo;
    m_CreateInfo.flags = {};
    m_CreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    m_CreateInfo.clipped = VK_TRUE;

    bool surfaceSupported = gpu.device.getSurfaceSupportKHR(gpu.queueFamily, surface);

    if (!surfaceSupported) {
        throw std::runtime_error("Selected physical device does not support presenting to this surface");
    }


    if (!Recreate(size)) {
        throw std::runtime_error{ "Failed to create Vulkan Swapchain" };
    }
}

bool Swapchain::Recreate(glm::ivec2 size)
{
    m_ImageIndex.reset();
    if (size.x <= 0 || size.y <= 0) { return false; }

    auto const capabilities = m_GPU.device.getSurfaceCapabilitiesKHR(m_CreateInfo.surface);
    m_CreateInfo.imageExtent = GetImageExtent(capabilities, size);
    m_CreateInfo.minImageCount = GetImageCount(capabilities);
    m_CreateInfo.oldSwapchain = m_Swapchain ? *m_Swapchain : vk::SwapchainKHR{};
    m_CreateInfo.preTransform = capabilities.currentTransform;

    //since we only use one queue
    m_CreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    m_CreateInfo.queueFamilyIndexCount = 0;
    m_CreateInfo.pQueueFamilyIndices = nullptr;

    assert(m_CreateInfo.imageExtent.width > 0 && m_CreateInfo.imageExtent.height > 0 &&  m_CreateInfo.minImageCount >= minImages_v);

    m_Device.waitIdle();
    m_Swapchain = m_Device.createSwapchainKHRUnique(m_CreateInfo);

    PopulateImages();
    CreateImageViews();

    CreatePresentSemaphores();

    size = GetSize();
    //std::println("[lvk] Swapchain [{}x{}]", size.x, size.y);

    return true;
}

void Swapchain::PopulateImages()
{
    //avoiding a new vector every call
    std::uint32_t imageCount = 0;
    vk::Result result = m_Device.getSwapchainImagesKHR(*m_Swapchain, &imageCount, nullptr);
    RequireSuccess(result, "Failed to get Swapchain Images");

    m_Images.resize(imageCount);
    result = m_Device.getSwapchainImagesKHR(*m_Swapchain, &imageCount, m_Images.data());
    RequireSuccess(result, "Failed to get Swapchain Images");
}
constexpr auto subresourceRange_v = [] {
    vk::ImageSubresourceRange ret{};
    ret.setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
    return ret;
    }();
void Swapchain::CreateImageViews()
{
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.subresourceRange = subresourceRange_v;
    imageViewCreateInfo.format = m_CreateInfo.imageFormat;

    m_ImageViews.clear();
    m_ImageViews.reserve(m_Images.size());
    for (auto const image : m_Images) {
        imageViewCreateInfo.image = image;
        m_ImageViews.push_back(m_Device.createImageViewUnique(imageViewCreateInfo));
    }

}

void Swapchain::CreatePresentSemaphores()
{
    m_PresentSemaphores.clear();
    m_PresentSemaphores.resize(m_Images.size());
    for (auto& semaphore : m_PresentSemaphores) {
        semaphore = m_Device.createSemaphoreUnique({});
    }
}

vk::SurfaceFormatKHR Swapchain::GetSurfaceFormat(std::span<const vk::SurfaceFormatKHR> supported)
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

vk::Extent2D Swapchain::GetImageExtent(vk::SurfaceCapabilitiesKHR const& capabilities, glm::uvec2 const size)
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

std::uint32_t Swapchain::GetImageCount(vk::SurfaceCapabilitiesKHR const& capabilities)
{
    if (capabilities.maxImageCount < capabilities.minImageCount) {
        return std::max(minImages_v, capabilities.minImageCount);
    }
    return std::clamp(minImages_v, capabilities.minImageCount,
        capabilities.maxImageCount);
}

std::optional<RenderTarget> Swapchain::AcquireNextImage(vk::Semaphore const toSignal)
{
    assert(!m_ImageIndex);
    static constexpr auto timeout_v = std::numeric_limits<std::uint64_t>::max();

    auto imageIndex = std::uint32_t{};
    auto const result = m_Device.acquireNextImageKHR(*m_Swapchain, timeout_v, toSignal, {}, &imageIndex);
    if (NeedsRecreation(result)) {return {}; }

    m_ImageIndex = static_cast<std::size_t>(imageIndex);
    return RenderTarget{
      .image = m_Images.at(*m_ImageIndex),
      .imageView = *m_ImageViews.at(*m_ImageIndex),
      .extent = m_CreateInfo.imageExtent,
    };
}

bool Swapchain::NeedsRecreation(vk::Result const result) const
{
    switch (result) {
        case vk::Result::eSuccess:
        case vk::Result::eSuboptimalKHR: return false;
        case vk::Result::eErrorOutOfDateKHR: return true;
        default: break;
    }
    throw std::runtime_error{ "Swapchain Error" };
}

bool Swapchain::Present(vk::Queue const queue)
{
    auto const imageIndex = static_cast<std::uint32_t>(m_ImageIndex.value());
    auto const waitSemaphore = *m_PresentSemaphores.at(static_cast<std::size_t>(imageIndex));

    std::array<vk::SwapchainKHR, 1> swapchains = { *m_Swapchain };
    std::array<uint32_t, 1> indices = { imageIndex };
    std::array<vk::Semaphore, 1> waitSemaphores = { waitSemaphore };

    vk::PresentInfoKHR presentInfo{};
    presentInfo.swapchainCount = static_cast<uint32_t>(swapchains.size());
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = indices.data();
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    auto const result = queue.presentKHR(presentInfo);
    m_ImageIndex.reset();
    return !NeedsRecreation(result);
}

vk::ImageMemoryBarrier2 Swapchain::GetBaseBarrier() const
{
    auto ret = vk::ImageMemoryBarrier2{};
    ret.image = m_Images.at(m_ImageIndex.value());
    ret.subresourceRange = subresourceRange_v;
    ret.srcQueueFamilyIndex = m_GPU.queueFamily;
    ret.dstQueueFamilyIndex = m_GPU.queueFamily;
    return ret;
}

Swapchain::~Swapchain() {
    m_Device.waitIdle();

    m_PresentSemaphores.clear();
    m_ImageViews.clear();
    m_Swapchain.reset();
}
