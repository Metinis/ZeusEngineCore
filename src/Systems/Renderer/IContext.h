#pragma once
#include "IResourceManager.h"

struct GLFWwindow;

namespace ZEN {
    enum eDepthModes {
        LESS,
        LEQUAL
    };
    struct MeshDrawableComp;
    class IContext {
    public:
        virtual ~IContext() = default;
        virtual void drawMesh(IResourceManager& resourceManager, const MeshDrawableComp& drawable) = 0;
        virtual void clear(bool shouldClearColor, bool shouldClearDepth) = 0;
        virtual void depthMask(bool val) = 0;
        virtual void enableCullFace() = 0;
        virtual void disableCullFace() = 0;
        virtual void setDepthMode(eDepthModes depthMode) = 0;
        virtual void setViewport(uint32_t width, uint32_t height) = 0;
        virtual void swapBuffers() = 0;
        static std::unique_ptr<IContext> create(eRendererAPI api,
            GLFWwindow* window);
    };
}
