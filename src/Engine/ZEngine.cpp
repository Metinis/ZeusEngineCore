#include "ZeusEngineCore/ZEngine.h"
#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/ModelLibrary.h"
#include "ZeusEngineCore/ModelImporter.h"
#include "ZeusEngineCore/Scene.h"
#include "ZeusEngineCore/Application.h"

using namespace ZEN;
ZEngine::ZEngine(eRendererAPI api, GLFWwindow* nativeWindow, const std::string& resourceRoot) : m_API(api) {
    m_Scene = std::make_unique<Scene>();
    m_Renderer = new Renderer(m_API, resourceRoot, nativeWindow);
    m_ModelLibrary = std::make_unique<ModelLibrary>(m_Renderer->getResourceManager(), resourceRoot);
    m_ModelImporter = std::make_unique<ModelImporter>(m_Scene.get(), m_Renderer->getResourceManager(), m_ModelLibrary.get());

    m_RenderSystem = new RenderSystem(m_Renderer, m_Scene.get(), m_ModelLibrary.get());
    m_CameraSystem = new CameraSystem(m_Scene.get());
    m_CameraSystem->setSize(Application::get().getWindow()->getWidth(), Application::get().getWindow()->getHeight());

    Application::get().pushLayer(m_Renderer);
    Application::get().pushLayer(m_RenderSystem);
    Application::get().pushLayer(m_CameraSystem);
}

ZEngine::~ZEngine() = default;
