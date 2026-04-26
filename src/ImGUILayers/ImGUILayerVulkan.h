#pragma once

namespace ZEN {
    struct EngineContext;
    class ImGUILayerVulkan{
    public:
        ImGUILayerVulkan();
        void init(EngineContext* ctx);
        ~ImGUILayerVulkan();
        void beginFrame();
        void render();
        void endFrame();
    };
}
