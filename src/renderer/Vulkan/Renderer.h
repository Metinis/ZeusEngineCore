
#pragma once
#include "ZeusEngineCore/IRenderer.h"
#include "Backend/APIBackend.h"
#include "Backend/DescriptorBuffer.h"
#include "Texture.h"
#include "Backend/APIRenderer.h"
#include <optional>

namespace ZEN::VKAPI {
    class Renderer : public IRenderer {
    public:
        void Init(RendererInitInfo &initInfo) override;

        ~Renderer() override;

        bool BeginFrame() override;

        void Submit(const glm::mat4 &transform, const std::shared_ptr<Material> &material,
                    const std::shared_ptr<IMesh> &mesh) override;

        void EndFrame(const std::function<void(void*)>& uiExtraDrawCallback = nullptr) override;


        void *
        GetCurrentCommandBuffer() override { return reinterpret_cast<void *>(static_cast<VkCommandBuffer>(m_CommandBuffer)); }

        RendererContextVariant GetContext() const override;

        ShaderInfoVariant GetShaderInfo() const override;

        VKAPI::APIRenderer* GetAPIRenderer() const override;

    private:
        void UpdateView();

        std::unique_ptr<VKAPI::APIBackend> m_Backend;
        std::unique_ptr<VKAPI::APIRenderer> m_APIRenderer;
        std::optional<DescriptorBuffer> m_ViewUBO{};
        Texture m_Texture;
        vk::CommandBuffer m_CommandBuffer;
    };
}
