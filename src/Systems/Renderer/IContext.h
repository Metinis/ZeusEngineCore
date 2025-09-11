
#pragma once
#include "IResourceManager.h"

struct GLFWwindow;

namespace ZEN {
    class IContext {
    public:
        virtual ~IContext() = default;
        virtual IResourceManager& getResourceManager() = 0;
        virtual void drawMesh(const MeshDrawableComp& meshRenderable) = 0;
        virtual void clear(bool shouldClearColor, bool shouldClearDepth) = 0;
        virtual void depthMask(bool val) = 0;
        virtual void swapBuffers() = 0;
        static std::unique_ptr<IContext> create(eRendererAPI api, GLFWwindow* window);
    };
}
