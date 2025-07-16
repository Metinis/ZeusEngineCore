#pragma once
#include "APIBackend.h"
#include "ZeusEngineCore/IRendererAPI.h"

namespace ZEN::VKAPI { //Handles all logic involving rendering/command buffer such as
    class Texture;
    // binding/drawing
    class APIRenderer : public IRendererAPI {
    public:
        explicit APIRenderer(VKAPI::APIBackend* apiBackend);

        bool BeginFrame() override;
        void DrawWithCallback(const std::function<void(void*)>& uiExtraDrawCallback) override;
        void SubmitAndPresent() override;
        //vulkan specific
        [[nodiscard]] std::size_t GetFrameIndex() const {return m_FrameInfo.sync->GetFrameIndex();}
        void SetUBO(const DescriptorBuffer& ubo);
        void SetImage(const Texture& texture);
        void DrawIndexed(vk::Buffer buffer) const; //todo api agnostic buffer
        void BindShader(vk::Pipeline pipeline);
        void SetPolygonMode(vk::PolygonMode mode) const {m_CommandBuffer.setPolygonModeEXT(mode);}
        void SetLineWidth(float width) const {m_CommandBuffer.setLineWidth(width);}
        void BindDescriptorSets();
    private:
        //vulkan specific
        bool AcquireRenderTarget();
        void TransitionForRender();
        void TransitionForPresent() const;

        vk::CommandBuffer m_CommandBuffer; //command buffer in use
        RenderFrameInfo m_FrameInfo;
        vk::RenderingInfo m_RenderingInfo{};
        APIBackend* m_Backend;
        std::optional<RenderTarget> m_RenderTarget{};
    };
}
