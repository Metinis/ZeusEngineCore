#pragma once
#include <vulkan/vulkan.hpp>

class VulkanCommandBlock {
public:
	VulkanCommandBlock(const vk::Device device, const vk::Queue queue,
		const vk::CommandPool commandPool);
	vk::CommandBuffer GetCommandBuffer() const { return *m_CommandBuffer; }
	void submitAndWait();
private:
	vk::Device m_Device{};
	vk::Queue m_Queue{};
	vk::UniqueCommandBuffer m_CommandBuffer;
};