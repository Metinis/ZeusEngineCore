#include "ZeusEngineCore/ZEngine.h"
#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/AssetLibrary.h"
#include "ZeusEngineCore/ModelImporter.h"
#include "ZeusEngineCore/Scene.h"
#include "ZeusEngineCore/Application.h"

using namespace ZEN;
ZEngine::ZEngine(eRendererAPI api, GLFWwindow* nativeWindow, const std::string& resourceRoot) : m_API(api) {
    m_Scene = new Scene();
    m_Renderer = new Renderer(m_API, resourceRoot, nativeWindow);
    Project::getActive()->init(m_Renderer->getResourceManager(), resourceRoot);
    m_ModelImporter = std::make_unique<ModelImporter>(m_Scene, m_Renderer->getResourceManager());
    m_RenderSystem = new RenderSystem(m_Renderer, m_Scene);
    m_CameraSystem = new CameraSystem(m_Scene);

    Application::get().pushLayer(m_Scene);
    Application::get().pushLayer(m_Renderer);
    Application::get().pushLayer(m_RenderSystem);
    Application::get().pushLayer(m_CameraSystem);
}

ZEngine::~ZEngine() = default;

void ZEngine::setAspectRatio(float aspectRatio) {
    m_Renderer->setAspectRatio(aspectRatio);
    m_Renderer->setSize(Application::get().getWindow()->getHandleWidth(),
        Application::get().getWindow()->getHandleHeight());
    m_CameraSystem->setAspectRatio(aspectRatio);
    std::cout<<"Aspect Ratio: "<<aspectRatio<<"\n";
}
