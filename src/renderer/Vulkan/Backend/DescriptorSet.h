#pragma once
#include <vulkan/vulkan.hpp>
#include "ZeusEngineCore/EngineConstants.h"
namespace ZEN::VKAPI {
    class DescriptorSet {
    public:
        explicit DescriptorSet(vk::Device device);

        DescriptorSet(DescriptorSet &&) noexcept = default;

        DescriptorSet &operator=(DescriptorSet &&) noexcept = default;

        DescriptorSet(const DescriptorSet &) = delete;

        DescriptorSet &operator=(const DescriptorSet &) = delete;

        void SetUBO(std::size_t frameIndex, const vk::DescriptorBufferInfo& descBufferInfo); //APIRenderer references this

        void SetImage(std::size_t frameIndex, const vk::DescriptorImageInfo& descImageInfo); //APIRenderer references this

        void SetSSBO(std::size_t frameIndex, const vk::DescriptorBufferInfo& descBufferInfo); //APIRenderer references this

        void BindDescriptorSets(vk::CommandBuffer commandBuffer, std::size_t frameIndex) const;

        [[nodiscard]] const std::vector<vk::DescriptorSetLayout> &GetSetLayouts() const { return m_SetLayoutViews; }

        const vk::PipelineLayout GetPipelineLayout() const { return *m_PipelineLayout; }

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
        Buffered <std::vector<vk::DescriptorSet>> m_DescriptorSets{};

    };
}