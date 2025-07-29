#pragma once
#include <memory>
#include <functional>

namespace ZEN {
    class IMesh;
    class IRendererBackend;

    enum class eRendererAPI {
        Vulkan,
        OpenGL
    };
    class IRendererAPI {
    public:
        virtual ~IRendererAPI() = default;
        virtual bool BeginFrame() = 0;
        virtual void DrawWithCallback(const std::function<void(void*)>& uiExtraDrawCallback) = 0;
        virtual void SubmitAndPresent() = 0;
        virtual void SetDepth(bool isDepth) = 0;
        virtual void SetMSAA(int msaa) = 0;
        virtual void Clear(bool shouldClearColor, bool shouldClearDepth) = 0;
        int GetMSAA() const { return m_MSAA; }
        virtual int GetMaxMSAA() = 0;
        static std::unique_ptr<IRendererAPI> Create(eRendererAPI api, IRendererBackend* apiBackend);

    protected:
        int m_MSAA{};
    };


}
