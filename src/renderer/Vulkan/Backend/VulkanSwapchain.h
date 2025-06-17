#pragma once
#include "VulkanDevice.h"
class VulkanSwapchain {
public:
	explicit VulkanSwapchain(vk::Device device, GPU const& gpu, vk::SurfaceKHR surface, 
		const DispatchLoaderDynamic& dynamicLoader, glm::ivec2 size);

	bool recreate(glm::ivec2 size);
	glm::ivec2 getSize() const { return { m_CreateInfo.imageExtent.width, m_CreateInfo.imageExtent.height }; };

private:
	using Swapchain = vk::UniqueHandle<vk::SwapchainKHR, DispatchLoaderDynamic>;
	void populateImages();
	void createImageViews();
	vk::SurfaceFormatKHR getSurfaceFormat(std::span<const vk::SurfaceFormatKHR> supported);
	vk::Extent2D getImageExtent(vk::SurfaceCapabilitiesKHR const& capabilities, glm::uvec2 const size);
	std::uint32_t getImageCount(vk::SurfaceCapabilitiesKHR const& capabilities);

	vk::Device m_Device;
	GPU m_GPU{};
	DispatchLoaderDynamic m_DynamicLoader;

	vk::SwapchainCreateInfoKHR m_CreateInfo{};
	Swapchain m_Swapchain{};
	std::vector<vk::Image> m_Images;
	std::vector<vk::UniqueImageView> m_ImageViews{};

	inline static constexpr auto srgbFormats_v = std::array{
		vk::Format::eR8G8B8A8Srgb,
		vk::Format::eB8G8R8A8Srgb,
	};
	inline static constexpr std::uint32_t minImages_v{ 3 };

};