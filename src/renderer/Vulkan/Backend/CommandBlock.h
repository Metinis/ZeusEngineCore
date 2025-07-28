#pragma once
#include <vulkan/vulkan.hpp>

namespace ZEN::VKAPI {
    class CommandBlock {
    public:
        CommandBlock(vk::Device device, vk::Queue queue,
                     vk::CommandPool commandPool);

        [[nodiscard]] vk::CommandBuffer GetCommandBuffer() const { return *m_CommandBuffer; }

        void submitAndWait();

    private:
        vk::Device m_Device{};
        vk::Queue m_Queue{};
        vk::UniqueCommandBuffer m_CommandBuffer;
    };
}