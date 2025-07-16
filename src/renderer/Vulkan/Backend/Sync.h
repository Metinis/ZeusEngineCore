#pragma once
#include <vulkan/vulkan.hpp>
#include "ZeusEngineCore/EngineConstants.h"

namespace ZEN::VKAPI {
    struct GPU;
    struct RenderSync {
        vk::UniqueSemaphore draw{};

        vk::UniqueFence drawn{};

        vk::CommandBuffer commandBuffer{};
    };

    class Sync {
    public:
        explicit Sync(const GPU &gpu, vk::Device device);

        RenderSync &GetRenderSyncAtFrame() { return m_RenderSync.at(m_FrameIndex); }

        void NextFrameIndex();

        [[nodiscard]] std::size_t GetFrameIndex() const { return m_FrameIndex; }

    private:
        vk::UniqueCommandPool m_RenderCmdPool{};
        Buffered<RenderSync> m_RenderSync{};
        std::size_t m_FrameIndex{};


    };
}