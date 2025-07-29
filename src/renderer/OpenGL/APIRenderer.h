#pragma once
#include "ZeusEngineCore/IRendererAPI.h"
#include <glad/glad.h>

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
    private:
        void SwapBuffers();

        GLuint m_FBO;
        GLuint m_DepthBuffer;
        GLuint m_FBOTex;

        GLuint m_MSAAFBO;
        GLuint m_MSAADepthBuffer;
        GLuint m_MSAATex;
        
        APIBackend* m_Backend;
    };
}

