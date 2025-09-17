#include "ZeusEngineCore/Renderer.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "ZeusEngineCore/InputEvents.h"

using namespace ZEN;

Renderer::Renderer(eRendererAPI api, GLFWwindow* window, entt::dispatcher& dispatcher) {
    m_Context = IContext::create(api, window);
    m_ResourceManager = IResourceManager::create(api);
    m_ViewUBO.uboID = m_ResourceManager->createUBO(0);
    m_InstanceUBO.uboID = m_ResourceManager->createUBO(1);
    m_GlobalUBO.uboID = m_ResourceManager->createUBO(2);
    m_MaterialUBO.uboID = m_ResourceManager->createUBO(3);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    m_MainFBO.fboID = m_ResourceManager->createFBO();
    m_ResourceManager->bindFBO(m_MainFBO.fboID);
    m_ColorTex.textureID = m_ResourceManager->createColorTex(fbWidth, fbHeight);
    m_DepthTex.textureID = m_ResourceManager->createDepthBuffer(fbWidth, fbHeight);

    dispatcher.sink<WindowResizeEvent>().connect<&Renderer::onResize>(*this);
}

void Renderer::beginFrame() {
    m_ResourceManager->bindFBO(m_MainFBO.fboID);
    m_Context->clear(true, true);

    if(m_Resized) {
        m_ResourceManager->bindFBO(m_MainFBO.fboID);
        m_ResourceManager->deleteTexture(m_ColorTex.textureID);
        m_ResourceManager->deleteDepthBuffer(m_DepthTex.textureID);
        m_ColorTex.textureID = m_ResourceManager->createColorTex(m_Width, m_Height);
        m_DepthTex.textureID = m_ResourceManager->createDepthBuffer(m_Width, m_Height);
        m_Resized = false;
    }
}

void Renderer::bindDefaultFBO() {
    m_ResourceManager->bindFBO(0);
}

void Renderer::endFrame() {
    m_Context->swapBuffers();
}

void Renderer::setDefaultShader(const MaterialComp &shader) {
    m_DefaultShader = shader;
}

void Renderer::onResize(const WindowResizeEvent &e) {
    std::cout<<"Renderer resized! "<<e.width<<"x "<<e.height<<"y \n";
    m_Resized = true;
    m_Width = e.width;
    m_Height = e.height;
}

