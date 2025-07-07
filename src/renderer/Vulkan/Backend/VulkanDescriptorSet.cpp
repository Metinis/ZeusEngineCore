#include "VulkanDescriptorSet.h"

VulkanDescriptorSet::VulkanDescriptorSet(const vk::Device device) : m_Device(device)
{
	CreateDescriptorPool();
	CreatePipelineLayout();
	CreateDescriptorSets();
}

void VulkanDescriptorSet::BindDescriptorSets(vk::CommandBuffer const commandBuffer, 
	std::size_t frameIndex, const vk::DescriptorBufferInfo& descInfo) const
{
	auto writes = std::array<vk::WriteDescriptorSet, 1>{};
	auto const& descriptorSets = m_DescriptorSets.at(frameIndex);
	auto const set0 = descriptorSets[0];
	auto write = vk::WriteDescriptorSet{};
	auto const viewUBOInfo = descInfo;
	write.setBufferInfo(viewUBOInfo)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setDstSet(set0)
		.setDstBinding(0);
	writes[0] = write;
	m_Device.updateDescriptorSets(writes, {});

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		*m_PipelineLayout, 0, descriptorSets,
		{});
}

void VulkanDescriptorSet::CreateDescriptorPool()
{
	static constexpr auto poolSizes_v = std::array{
		// 2 uniform buffers
		vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 2},
	};
	vk::DescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.setPoolSizes(poolSizes_v).setMaxSets(16);
	m_DescriptorPool = m_Device.createDescriptorPoolUnique(poolCreateInfo);
}
//simplifies binding creation
constexpr auto layoutBinding(std::uint32_t binding, vk::DescriptorType const type) {
	return vk::DescriptorSetLayoutBinding{ binding, type, 1, vk::ShaderStageFlagBits::eAllGraphics };
}
void VulkanDescriptorSet::CreatePipelineLayout()
{
	static constexpr auto set0Bindings_v = std::array{ 
		layoutBinding(0, vk::DescriptorType::eUniformBuffer), };

	auto setLayoutCreateInfos = std::array<vk::DescriptorSetLayoutCreateInfo, 1>{};
	setLayoutCreateInfos[0].setBindings(set0Bindings_v);

	for (auto const& setLayoutCreateInfo : setLayoutCreateInfos) {
		m_SetLayouts.push_back(m_Device.createDescriptorSetLayoutUnique(setLayoutCreateInfo));
		m_SetLayoutViews.push_back(*m_SetLayouts.back());
	}
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.setSetLayouts(m_SetLayoutViews);
	m_PipelineLayout = m_Device.createPipelineLayoutUnique(pipelineLayoutCreateInfo);
}

void VulkanDescriptorSet::CreateDescriptorSets()
{
	for (auto& descriptorSets : m_DescriptorSets) {
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.setDescriptorPool(*m_DescriptorPool);
		allocInfo.setSetLayouts(m_SetLayoutViews);
		descriptorSets = m_Device.allocateDescriptorSets(allocInfo);
	}
}
