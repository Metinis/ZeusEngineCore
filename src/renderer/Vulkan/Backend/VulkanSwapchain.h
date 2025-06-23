#pragma once
#include "VulkanDevice.h"

struct RenderTarget {
	vk::Image image{};
	vk::ImageView imageView{};
	vk::Extent2D extent{};
};

class VulkanSwapchain {
public:
	explicit VulkanSwapchain(vk::Device device, GPU const& gpu, vk::SurfaceKHR surface, glm::ivec2 size);
    ~VulkanSwapchain();

	bool Recreate(glm::ivec2 size);
	glm::ivec2 GetSize() const { return { m_CreateInfo.imageExtent.width, m_CreateInfo.imageExtent.height }; };
	std::optional<RenderTarget> AquireNextImage(vk::Semaphore const toSignal);
	vk::ImageMemoryBarrier2 GetBaseBarrier() const;
	vk::Semaphore GetPresentSemaphore() const { return *m_PresentSemaphores.at(m_ImageIndex.value()); };
	vk::Format GetFormat() const { return m_CreateInfo.imageFormat; }
	bool Present(vk::Queue const queue);

private:
	void PopulateImages();
	void CreateImageViews();
	void CreatePresentSemaphores();
	bool NeedsRecreation(vk::Result const result) const;
	vk::SurfaceFormatKHR GetSurfaceFormat(std::span<const vk::SurfaceFormatKHR> supported);
	vk::Extent2D GetImageExtent(vk::SurfaceCapabilitiesKHR const& capabilities, glm::uvec2 const size);
	std::uint32_t GetImageCount(vk::SurfaceCapabilitiesKHR const& capabilities);


	

	vk::Device m_Device;
	GPU m_GPU{};
	std::vector<vk::UniqueSemaphore> m_PresentSemaphores{};
	vk::SwapchainCreateInfoKHR m_CreateInfo{};
	vk::UniqueSwapchainKHR m_Swapchain{};
	std::vector<vk::Image> m_Images;
	std::vector<vk::UniqueImageView> m_ImageViews{};
	std::optional<std::size_t> m_ImageIndex{};

	inline static constexpr auto srgbFormats_v = std::array{
		vk::Format::eR8G8B8A8Srgb,
		vk::Format::eB8G8R8A8Srgb,
	};
	inline static constexpr std::uint32_t minImages_v{ 3 };

};