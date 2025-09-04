#include "ZeusEngineCore/Renderer.h"

using namespace ZEN;

Renderer::Renderer(eRendererAPI api, GLFWwindow* window) {
    m_Context = IContext::create(api, window);
    m_ViewUBO.uboID = m_Context->getResourceManager().createUBO(0);
    m_InstanceUBO.uboID = m_Context->getResourceManager().createUBO(1);
    m_GlobalUBO.uboID = m_Context->getResourceManager().createUBO(2);
    m_MaterialUBO.uboID = m_Context->getResourceManager().createUBO(3);
}

void Renderer::beginFrame() {
    m_Context->clear(true, true);
}

void Renderer::endFrame() {
    m_Context->swapBuffers();
}

void Renderer::setDefaultShader(const MaterialComp &shader) {
    m_DefaultShader = shader;
}
