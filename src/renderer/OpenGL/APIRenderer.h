#pragma once
#include "ZeusEngineCore/IRendererAPI.h"

namespace ZEN::OGLAPI {
    class APIBackend;
    class APIRenderer : public IRendererAPI {
    public:
        APIRenderer(OGLAPI::APIBackend* apiBackend){};
        ~APIRenderer() override = default;
        bool BeginFrame() override;
        void DrawWithCallback(const std::function<void(void*)>& uiExtraDrawCallback) override;
        void SubmitAndPresent() override;
    private:
    };
}

