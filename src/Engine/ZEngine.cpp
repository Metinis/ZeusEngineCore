#include "ZeusEngineCore/ZEngine.h"
#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/ModelLibrary.h"
#include "ZeusEngineCore/ModelImporter.h"
#include "ZeusEngineCore/Scene.h"
#include "ZeusEngineCore/EventDispatcher.h"

#include <memory>

using namespace ZEN;
ZEngine::ZEngine(eRendererAPI api, GLFWwindow* nativeWindow, const std::string& resourceRoot) : m_API(api){
    m_Dispatcher = std::make_unique<EventDispatcher>();
    m_Scene = std::make_unique<Scene>(m_Dispatcher.get());
    m_Renderer = std::make_unique<Renderer>(m_API, resourceRoot, nativeWindow, *m_Dispatcher);
    m_ModelLibrary = std::make_unique<ModelLibrary>(m_Dispatcher.get(), m_Renderer->getResourceManager(), resourceRoot);
    m_ModelImporter = std::make_unique<ModelImporter>(m_Scene.get(), m_Renderer->getResourceManager(), m_ModelLibrary.get());
    m_RenderSystem = std::make_unique<RenderSystem>(m_Renderer.get(), m_Scene.get(), m_ModelLibrary.get(), m_Dispatcher.get());
    m_CameraSystem = std::make_unique<CameraSystem>(m_Scene.get(), m_Dispatcher.get());

}

ZEngine::~ZEngine() = default;

void ZEngine::onUpdate(float deltaTime) {
    m_CameraSystem->onUpdate(deltaTime);
    m_RenderSystem->onUpdate();
}

void ZEngine::onRender(const std::function<void()>& uiRender) {
    m_Renderer->beginFrame();
    m_RenderSystem->onRender();
    m_Renderer->bindDefaultFBO();
    if(uiRender) {
        uiRender();
    }
    m_Renderer->endFrame();
}
