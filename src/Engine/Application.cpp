
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

void Application::run() {
    for(Layer* layer : m_LayerStack) {
        layer->onUpdate();
    }
    for(Layer* layer : m_LayerStack) {
        //begin imgui frame
        layer->onUIRender();
        //end imgui frame
    }
}
