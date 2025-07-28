#include "Sync.h"
#include "Device.h"
#include <ranges>
using namespace ZEN::VKAPI;

Sync::Sync(const GPU& gpu, const vk::Device device)
{
	vk::CommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPoolInfo.queueFamilyIndex = gpu.queueFamily;
	m_RenderCmdPool = device.createCommandPoolUnique(commandPoolInfo);

	vk::CommandBufferAllocateInfo commandBufferAllInfo{};
	commandBufferAllInfo.setCommandPool(*m_RenderCmdPool);
	commandBufferAllInfo.setCommandBufferCount(static_cast<std::uint32_t>(buffering_v));
	commandBufferAllInfo.level = vk::CommandBufferLevel::ePrimary;

	auto const command_buffers = device.allocateCommandBuffers(commandBufferAllInfo);
	assert(command_buffers.size() == m_RenderSync.size());

	static constexpr vk::FenceCreateInfo fenceCreateInfo_v{ vk::FenceCreateFlagBits::eSignaled };

	for (auto [sync, command_buffer] : std::views::zip(m_RenderSync, command_buffers)) {
		sync.commandBuffer = command_buffer;
		sync.draw = device.createSemaphoreUnique({});
		sync.drawn = device.createFenceUnique(fenceCreateInfo_v);
	}
}

void Sync::NextFrameIndex()
{
	m_FrameIndex = (m_FrameIndex + 1) % m_RenderSync.size();
}
