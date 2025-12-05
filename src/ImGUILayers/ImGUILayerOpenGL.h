#pragma once
#include "ImGUILayer.h"
#include "GLFW/glfw3.h"

namespace ZEN {
    class ImGUILayerOpenGL : public ImGUILayer {
    public:
        ImGUILayerOpenGL(GLFWwindow* window);

        ~ImGUILayerOpenGL() override;

        void beginFrame() override;

        void render() override;

        //command buffer ignored
        void endFrame(void *commandBuffer) override;
    };
}
