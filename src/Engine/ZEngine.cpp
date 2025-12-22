#include "ZeusEngineCore/ZEngine.h"
#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/ModelImporter.h"
#include "ZeusEngineCore/Scene.h"
#include "ZeusEngineCore/Application.h"

using namespace ZEN;
ZEngine::ZEngine() {

}

void ZEngine::init() {
    m_Scene = new Scene();
    m_Renderer = new Renderer();
    Project::getActive()->init();
    m_ModelImporter = std::make_unique<ModelImporter>();
    m_RenderSystem = new RenderSystem();
    m_CameraSystem = new CameraSystem();

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
