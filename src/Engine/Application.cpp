#include "ZeusEngineCore/core/Application.h"

#include "ImGuizmo.h"
#include "LayerStack.h"
#include "glm/gtc/type_ptr.hpp"
#include "ZeusEngineCore/engine/Scene.h"
#include "ZeusEngineCore/core/InputEvents.h"
#include "ZeusEngineCore/core/Event.h"
#include "ZeusEngineCore/engine/CameraSystem.h"
#include "ZeusEngineCore/core/Project.h"

using namespace ZEN;

Application* Application::s_Instance = nullptr;

Application::Application() {
    s_Instance = this;
}

Application::~Application() {
}

void Application::init() {
    m_Window = std::make_unique<Window>("Zeus Editor");
    m_LayerStack = std::make_unique<LayerStack>();
    Project::createNew();

    m_Engine = std::make_unique<ZEngine>();
    m_Engine->init();

    m_Window->attachDispatcher();

    m_ImGUILayer = ImGUILayer::create(m_Window->getNativeWindow(), m_API);
    m_Running = true;

    m_Engine->setAspectRatio(m_Window->getHandleWidth() / m_Window->getHandleHeight());

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
    m_Engine->getRenderer().setSize(event.getWidth(), event.getHeight());
    return true;
}

void Application::close() {
    m_Running = false;
}
void Application::run() {
    while(m_Running && !m_Window->shouldClose()) {

        //---------------UPDATE LOGIC-------------------
        m_LayerStack->flush();
        m_Window->pollEvents();
        const float dt = m_Window->getDeltaTime();

        for(Layer* layer : *m_LayerStack) {
            layer->onUpdate(dt);
        }
        //----------------------------------------------


        //---------------RENDER LOGIC-------------------
        m_Engine->getRenderer().beginFrame();
        for(Layer* layer : *m_LayerStack) {
            layer->onRender();
        }
        m_Engine->getRenderer().bindDefaultFBO();


        m_ImGUILayer->beginFrame();

        for(Layer* layer : *m_LayerStack) {

            layer->onUIRender();
        }
        m_ImGUILayer->render();
        m_ImGUILayer->endFrame(nullptr);

        m_Engine->getRenderer().endFrame();
        //----------------------------------------------

    }
}
