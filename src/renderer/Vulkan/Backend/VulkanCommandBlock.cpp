#include "VulkanCommandBlock.h"
#include <cstdint>

VulkanCommandBlock::VulkanCommandBlock(const vk::Device device, const vk::Queue queue, 
	const vk::CommandPool commandPool) : m_Device(device), m_Queue(queue)
{
	vk::CommandBufferAllocateInfo allocateInfo{};
	allocateInfo.setCommandPool(commandPool);
	allocateInfo.setCommandBufferCount(1);
	allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

	auto commandBuffers = m_Device.allocateCommandBuffersUnique(allocateInfo);

	//move front of commandBuffers since we specified only 1 in allocate info
	m_CommandBuffer = std::move(commandBuffers.front());

	//start recording commands as soon as created
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	m_CommandBuffer->begin(beginInfo);

}

void VulkanCommandBlock::submitAndWait()
{
	if (!m_CommandBuffer) { return; }

	//end the recording and submit
	m_CommandBuffer->end();
	vk::SubmitInfo2KHR submitInfo{};
	vk::CommandBufferSubmitInfo commandBufferInfo{*m_CommandBuffer};
	submitInfo.setCommandBufferInfos(commandBufferInfo);
	vk::UniqueFence fence = m_Device.createFenceUnique({});
	m_Queue.submit2(submitInfo, *fence);

	//wait for submit fence
	static constexpr std::uint64_t fenceTimeout_v = 3'000'000'000ull; //3 sec

	vk::Result result = m_Device.waitForFences(*fence, vk::True, fenceTimeout_v);
	if (result != vk::Result::eSuccess) {
		//todo use printlns
		std::printf("Failed to submit command buffer!");
	}
	m_CommandBuffer.reset();
}
