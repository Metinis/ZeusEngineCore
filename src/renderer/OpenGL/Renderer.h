#pragma once
#include "ZeusEngineCore/IRenderer.h"

namespace ZEN::OGLAPI {
    class Renderer : public IRenderer {
    public:
        void Init(RendererInitInfo &initInfo) override;

        ~Renderer() override;

        bool BeginFrame() override;

        void Submit(const glm::mat4 &transform, const std::shared_ptr<Material> &material,
                    const std::shared_ptr<IMesh> &mesh) override;

        void EndFrame(const std::function<void(void*)>& uiExtraDrawCallback = nullptr) override;

        void *GetCurrentCommandBuffer() override { return nullptr; }

        RendererContextVariant GetContext() const override { return ContextInfo{}; };

        ShaderInfoVariant GetShaderInfo() const override;

        VKAPI::APIRenderer* GetAPIRenderer() const override;
    };
}
