
#include "Application.h"

using namespace ZEN;
Application::Application() {
    m_LayerStack = std::make_unique<LayerStack>();
}

Application::~Application() {
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
        for(Layer* layer : *m_LayerStack) {
            layer->onUpdate();
        }
        for(Layer* layer : *m_LayerStack) {
            //begin imgui frame
            layer->onUIRender();
            //end imgui frame
        }
    }
}
