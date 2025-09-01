#include "ZeusEngineCore/Renderer.h"

using namespace ZEN;

Renderer::Renderer(eRendererAPI api, GLFWwindow* window) {
    m_Context = IContext::create(api, window);
}

void Renderer::beginFrame() {
    m_Context->clear(true, true);
}

void Renderer::endFrame() {
    m_Context->swapBuffers();
}
