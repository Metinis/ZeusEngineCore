#pragma once
#include "APIBackend.h"

namespace ZEN::VKAPI { //Handles all logic involving rendering/command buffer such as
    // binding/drawing
    class APIRenderer {
    public:
        explicit APIRenderer(APIBackend* apiBackend);
        void BeginFrame();
        void TransitionForRender() const;
        void TransitionForPresent() const;
        void SubmitAndPresent();
        bool AcquireRenderTarget();
        void Render(const std::function<void(vk::CommandBuffer, vk::Extent2D)> &drawCallback = nullptr,
               const std::function<void(vk::CommandBuffer)> &uiDrawCallback = nullptr);
        //vulkan specific
        [[nodiscard]] std::size_t GetFrameIndex() const {return m_FrameInfo.sync->GetFrameIndex();}
        void BindDescriptorSets(DescriptorBuffer &ubo, const vk::DescriptorImageInfo &descriptorImageInfo);
    private:

        vk::CommandBuffer m_CommandBuffer; //command buffer in use
        RenderFrameInfo m_FrameInfo;
        APIBackend* m_Backend;
        std::optional<RenderTarget> m_RenderTarget{};
    };
}
