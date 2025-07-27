#pragma once
#include <vulkan/vulkan.hpp>
#include "ZeusEngineCore/IRendererAPI.h"
#include "APIBackend.h"
#include "Image.h"

namespace ZEN::VKAPI { //Handles all logic involving rendering/command buffer such as
    class Texture;
    class DescriptorBuffer;
    // binding/drawing
    class APIRenderer : public IRendererAPI {
    public:
        explicit APIRenderer(VKAPI::APIBackend* apiBackend);

        bool BeginFrame() override;
        void DrawWithCallback(const std::function<void(void*)>& uiExtraDrawCallback) override;
        void SubmitAndPresent() override;
        void SetDepth(bool isDepth) override;
        void Clear(bool shouldClearColor, bool shouldClearDepth) override;
        //vulkan specific
        [[nodiscard]] std::size_t GetFrameIndex() const {return m_FrameInfo.sync->GetFrameIndex();}
        void SetUBO(const DescriptorBuffer& ubo);
        void SetSSBO(const DescriptorBuffer& ubo);
        void SetImage(const Texture& texture); //placeholder use desc info instead
        void DrawIndexed(vk::Buffer buffer, std::uint32_t instanceCount) const; //todo api agnostic buffer
        void BindShader(vk::Pipeline pipeline);
        void SetPolygonMode(vk::PolygonMode mode) const {m_CommandBuffer.setPolygonModeEXT(mode);}
        void SetLineWidth(float width) const {m_CommandBuffer.setLineWidth(width);}
        void BindDescriptorSets();
    private:
        //vulkan specific
        bool AcquireRenderTarget();
        void TransitionForRender();
        void TransitionForPresent();
        void SetBarriersForRender() const;
        void SetBarriersForPresent() const;
        void SetAttachments(bool shouldClearColor, bool shouldClearDepth, bool shouldUseDepth);

        bool m_IsDepth{true};

        vk::CommandBuffer m_CommandBuffer; //command buffer in use
        RenderFrameInfo m_FrameInfo;
        APIBackend* m_Backend;
        std::optional<RenderTarget> m_RenderTarget{};
        std::optional<Image> m_DepthImage{};
        std::optional<vk::UniqueImageView> m_DepthImageView{};
    };
}
