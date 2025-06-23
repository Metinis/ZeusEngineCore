#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanDevice.h"
#include "ZeusEngineCore/EngineConstants.h"


template <typename Type>
using Buffered = std::array<Type, buffering_v>;

struct RenderSync{
	vk::UniqueSemaphore draw{};

	vk::UniqueFence drawn{};

	vk::CommandBuffer commandBuffer{};
};

class VulkanSync {
public:
	explicit VulkanSync(const GPU& gpu, const vk::Device device);
	RenderSync& GetRenderSyncAtFrame() { return m_RenderSync.at(m_FrameIndex); }
	void NextFrameIndex();
	
private:
	vk::UniqueCommandPool m_RenderCmdPool{};
	Buffered<RenderSync> m_RenderSync{};
	std::size_t m_FrameIndex{};


};