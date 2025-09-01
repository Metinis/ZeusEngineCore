#include "ZeusEngineCore/Renderer.h"

using namespace ZEN;

Renderer::Renderer(GLFWwindow* window) : m_Context(window) {

}

void Renderer::beginFrame() {
    m_Context.clear(true, true);
}

void Renderer::endFrame() {
    m_Context.swapBuffers();
}
