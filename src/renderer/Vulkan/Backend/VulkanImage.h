#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include <cstdint>
#include <glm/vec2.hpp>
#include <functional>
#include <memory>
#include "VulkanBuffer.h"
#include "ZeusEngineCore/InfoVariants.h"

struct ImageHandle {
	VmaAllocator allocator{};
	VmaAllocation allocation{};
	vk::Image image{};
	vk::Extent2D extent{};
	vk::Format format{};
	std::uint32_t levels{};
};

struct ImageCreateInfo {
	VmaAllocator allocator;
	std::uint32_t queueFamily;
	std::shared_ptr<std::function<void(DeferredHandle)>> destroyCallback; //todo change name
};

struct Bitmap {
	std::span<std::byte const> bytes{};
	glm::ivec2 size{};
};

class VulkanImage {
public:
	VulkanImage(ImageCreateInfo const& createInfo, vk::ImageUsageFlags usage,
		std::uint32_t levels, vk::Format format, vk::Extent2D extent);
	// No copy
	VulkanImage(const VulkanImage&) = delete;
	VulkanImage& operator=(const VulkanImage&) = delete;

	// Move
	VulkanImage(VulkanImage&& other) noexcept {
		*this = std::move(other);
	}
	VulkanImage& operator=(VulkanImage&& other) noexcept;
	~VulkanImage() {
		Destroy();
	}
	vk::Image Get() const { return m_Handle.image; }
	std::uint32_t GetLevels() const { return m_Handle.levels; }
	vk::Format GetFormat() const { return m_Handle.format; }

private:
	void Destroy();

	std::function<void(DeferredHandle)> m_DestroyCallback;
	ImageHandle m_Handle{};
	
};