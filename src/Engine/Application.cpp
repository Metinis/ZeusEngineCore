
#include "ZeusEngineCore/Application.h"
#include "ZeusEngineCore/Scene.h"
#include "LayerStack.h"

using namespace ZEN;
Application::Application() {

}

Application::~Application() {
}

void Application::init() {
    m_Window = std::make_unique<Window>(1280, 720, "Zeus Editor", m_API);
    m_Engine = std::make_unique<ZEngine>(m_API, m_Window->getNativeWindow(), m_ResourceRoot);
    m_Window->attachDispatcher(m_Engine->getDispatcher());

    m_LayerStack = std::make_unique<LayerStack>();
    m_ImGUILayer = ImGUILayer::create(m_Window->getNativeWindow(), m_API);
    m_Running = true;

    m_Engine->getScene().createDefaultScene(m_Engine.get());


}

void Application::pushLayer(Layer* layer) {
    m_LayerStack->pushLayer(layer);
    layer->onAttach();
}

void Application::pushOverlay(Layer* layer) {
    m_LayerStack->pushOverlay(layer);
    layer->onAttach();
}

void Application::close() {
    m_Running = false;
}

void Application::run() {
    while(m_Running && !m_Window->shouldClose()) {
        m_Window->pollEvents();

        //todo change this
        const float dt = m_Window->getDeltaTime();
        m_Engine->onUpdate(dt);


        for(Layer* layer : *m_LayerStack) {
            layer->onUpdate();
        }
        auto uiRenderFunc = [this]() {
            m_ImGUILayer->beginFrame();
            for(Layer* layer : *m_LayerStack) {
                //begin imgui frame
                layer->onUIRender();
                //end imgui frame
            }
            m_ImGUILayer->render();
            m_ImGUILayer->endFrame(nullptr);
        };
        m_Engine->onRender(uiRenderFunc);


    }
}
