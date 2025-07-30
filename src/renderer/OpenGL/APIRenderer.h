#pragma once
#include "ZeusEngineCore/IRendererAPI.h"
//#include <glad/glad.h>

namespace ZEN::OGLAPI {
    class APIBackend;
    class APIRenderer : public IRendererAPI {
    public:
        explicit APIRenderer(OGLAPI::APIBackend* apiBackend);
        ~APIRenderer() override = default;
        bool BeginFrame() override;
        void DrawWithCallback(const std::function<void(void*)>& uiExtraDrawCallback) override;
        void SubmitAndPresent() override;
        void SetDepth(bool isDepth) override;
        void Clear(bool shouldClearColor, bool shouldClearDepth) override;
        void SetMSAA(int msaa) override;
        int GetMaxMSAA() override;
    private:
        void SwapBuffers();

        std::uint32_t m_MSAAFBO;
        std::uint32_t  m_MSAADepthBuffer;
        std::uint32_t  m_MSAATex;
        
        APIBackend* m_Backend;
    };
}

