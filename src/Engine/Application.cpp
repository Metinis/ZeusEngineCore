#include "ZeusEngineCore/core/Application.h"
#include "LayerStack.h"
#include "tinyfiledialogs.h"
#include "ImGUILayers/ImGUILayerVulkan.h"
#include "ZeusEngineCore/engine/Scene.h"
#include "ZeusEngineCore/core/InputEvents.h"
#include "ZeusEngineCore/core/Event.h"
#include "ZeusEngineCore/engine/CameraSystem.h"
#include "ZeusEngineCore/core/Project.h"
#include "ZeusEngineCore/engine/ModelImporter.h"
#include "ZeusEngineCore/engine/ZeusPhysicsSystem.h"

using namespace ZEN;

Application* Application::s_Instance = nullptr;

Application::Application() {
    s_Instance = this;

    m_Ctx.window = std::make_unique<Window>("Zeus Editor");
    m_Ctx.physicsSystem = new ZeusPhysicsSystem();
    m_Ctx.scene = new Scene();
    m_Ctx.cameraSystem = new CameraSystem();
    m_Ctx.modelImporter = std::make_unique<ModelImporter>();
    m_Ctx.systemManager = std::make_unique<SystemManager>();
    m_Ctx.compRegistry = std::make_unique<CompRegistry>();
    m_LayerStack = std::make_unique<LayerStack>();
    m_Ctx.vkRenderer = std::make_unique<VKRenderer>();
    Project::createNew();

    m_ImGUILayer = std::make_unique<ImGUILayerVulkan>();

    pushLayer(m_Ctx.cameraSystem);
    pushLayer(m_Ctx.scene);
    pushLayer(m_Ctx.physicsSystem);
}

Application::~Application() {
    Project::shutdown();
}

void Application::init() {
    spdlog::set_level(spdlog::level::debug);

    const char* title = "Select a project";
    const char* defaultPath = nullptr;
    const char* folderPath = tinyfd_selectFolderDialog(title, defaultPath);
    Project::getActive()->init(folderPath);
    m_Ctx.vkRenderer->init(&m_Ctx);
    Project::getActive()->getAssetLibrary()->init(&m_Ctx); //todo check this sketch

    m_Ctx.scene->init(&m_Ctx);

    m_Ctx.modelImporter->init(&m_Ctx);
    m_Ctx.cameraSystem->init(&m_Ctx);
    m_Ctx.physicsSystem->init(&m_Ctx);
    m_Ctx.systemManager->init(&m_Ctx);
    m_Ctx.compRegistry->init(&m_Ctx);

    m_Ctx.window->attachDispatcher(this);

    m_ImGUILayer->init(&m_Ctx);


    m_Ctx.scene->createDefaultScene();

    m_Running = true;
}

void Application::pushLayer(Layer* layer) {
    layer->onAttach();
    m_LayerStack->pushLayer(layer);
    
}

void Application::pushOverlay(Layer* layer) {
    layer->onAttach();
    m_LayerStack->pushOverlay(layer);
    
}

void Application::popLayer(Layer *layer) {
    layer->onDettach();
    m_LayerStack->popLayer(layer);
    
}

void Application::popOverlay(Layer *layer) {
    layer->onDettach();
    m_LayerStack->popOverlay(layer);
    
}

void Application::callEvent(Event &event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {return onWindowResize(e); });
    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayMode(e); });

    for (auto it = m_LayerStack->rbegin(); it != m_LayerStack->rend(); ++it)
    {
        if (event.handled)
            break;
        (*it)->onEvent(event);
    }
}

bool Application::onPlayMode(const RunPlayModeEvent &event) {
    if(event.getPlaying()) {
        m_IsPlaying = true;
    }
    return false;
}

bool Application::onWindowResize(const WindowResizeEvent &event) {
    //m_Engine->getRenderer().setSize(event.getWidth(), event.getHeight());
    return true;
}

void Application::close() {
    m_Running = false;
}
void Application::run() {
    while(m_Running && !m_Ctx.window->shouldClose()) {

        //---------------UPDATE LOGIC-------------------
        m_LayerStack->flush();
        m_Ctx.window->pollEvents();
        const float dt = m_Ctx.window->getDeltaTime();

        for(Layer* layer : *m_LayerStack) {
            layer->onUpdate(dt);
        }
        //----------------------------------------------

        //---------------RENDER LOGIC-------------------
        for(Layer* layer : *m_LayerStack) {
            layer->onRender();
        }

        m_ImGUILayer->beginFrame();
        for(Layer* layer : *m_LayerStack) {

            layer->onUIRender();
        }
        m_ImGUILayer->render();
        m_Ctx.vkRenderer->beginFrame();
        m_Ctx.vkRenderer->draw();
        m_Ctx.vkRenderer->endFrame();
        //----------------------------------------------

    }
}
