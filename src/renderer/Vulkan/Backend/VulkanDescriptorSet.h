#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanSync.h"
class VulkanDescriptorSet {
public:
	VulkanDescriptorSet(const vk::Device device);
	VulkanDescriptorSet(VulkanDescriptorSet&&) noexcept = default; 
	VulkanDescriptorSet& operator=(VulkanDescriptorSet&&) noexcept = default; 

	VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;  
	VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete; 
	void BindDescriptorSets(vk::CommandBuffer const commandBuffer, std::size_t frameIndex,
		const vk::DescriptorBufferInfo& descInfo) const;
	const std::vector<vk::DescriptorSetLayout>& GetSetLayouts() const { return m_SetLayoutViews; }
private:
	void CreateDescriptorPool();
	void CreatePipelineLayout();
	void CreateDescriptorSets();

	vk::Device m_Device;
	vk::UniqueDescriptorPool m_DescriptorPool{};
	std::vector<vk::UniqueDescriptorSetLayout> m_SetLayouts{};
	std::vector<vk::DescriptorSetLayout> m_SetLayoutViews{}; //copy of set layouts (not ref)
	vk::UniquePipelineLayout m_PipelineLayout{};
	//buffered since descriptor sets differ per frame
	Buffered<std::vector<vk::DescriptorSet>> m_DescriptorSets{};

};