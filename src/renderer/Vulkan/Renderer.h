
#pragma once
#include "ZeusEngineCore/IRenderer.h"
#include "Backend/Backend.h"
#include "Backend/DescriptorBuffer.h"
#include "Texture.h"
#include <optional>

namespace ZEN::VKAPI {
    class Renderer : public IRenderer {
    public:
        void Init(RendererInitInfo &initInfo) override;

        ~Renderer() override;

        bool BeginFrame() override;

        void Submit(const glm::mat4 &transform, const std::shared_ptr<Material> &material,
                    const std::shared_ptr<IMesh> &mesh) override;

        void EndFrame(const std::function<void(vk::CommandBuffer)> &uiExtraDrawCallback = nullptr) override;

        void DrawMesh(const IMesh &mesh, Material &material) override;

        void *
        GetCurrentCommandBuffer() override { return reinterpret_cast<void *>(static_cast<VkCommandBuffer>(m_CommandBuffer)); }

        RendererContextVariant GetContext() const override;

        ShaderInfoVariant GetShaderInfo() const override;

    private:
        void UpdateView();

        std::unique_ptr<VKAPI::Backend> m_VKBackend;
        std::optional<DescriptorBuffer> m_ViewUBO{};
        Texture m_Texture;
        vk::CommandBuffer m_CommandBuffer;
    };
}
