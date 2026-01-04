#include "ZeusEngineCore/ZEngine.h"
#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/ModelImporter.h"
#include "ZeusEngineCore/Scene.h"
#include "ZeusEngineCore/Application.h"
#include <tinyfiledialogs.h>

using namespace ZEN;
ZEngine::ZEngine() {

}

void ZEngine::init() {
    m_Renderer = new Renderer();
    //load project root through dialogue

    const char* title = "Select a project";
    const char* defaultPath = nullptr;
    const char* folderPath = tinyfd_selectFolderDialog(title, defaultPath);
    Project::getActive()->init(folderPath);

    m_SystemManager = std::make_unique<SystemManager>();
    m_CompRegistry = std::make_unique<CompRegistry>();

    m_Scene = new Scene();
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
