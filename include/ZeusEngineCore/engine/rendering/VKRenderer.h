#pragma once
#include "VKBackend.h"
#include "VKContext.h"
#include "VKResources.h"

namespace ZEN {
    struct EngineContext;
    struct MeshData;
    struct DrawCall;

    class ZEN_API VKRenderer {
    public:
        VKRenderer();
        void init(EngineContext* ctx);
        ImGui_ImplVulkan_InitInfo initImgui();

        void beginFrame();
        void draw();
        void endFrame();

        void submitDrawCall(const DrawCall& call);
        void setImGUIMode(bool mode);

        VKResources *getResources() {return &m_ResourceCtx;}

        void cleanup();
        ~VKRenderer();
    private:
        VKContext m_StateCtx{};
        RenderContext m_RenderCtx{};
        VKResources m_ResourceCtx{};
        //todo just pass a context struct rather than expose everything

    };
}
