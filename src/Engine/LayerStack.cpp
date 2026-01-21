#include "LayerStack.h"

using namespace ZEN;

LayerStack::LayerStack() {
    //m_LayerInsertIndex = m_Layers.begin();
}

LayerStack::~LayerStack() {
    for(const Layer* layer : m_Layers) {
        delete layer;
    }
}

void LayerStack::pushLayer(Layer* layer) {
    m_LayersToPushFront.push_back(layer);
}

void LayerStack::pushOverlay(Layer* overlay) {
    m_LayersToPushBack.push_back(overlay);
}

void LayerStack::popLayer(Layer *layer) {
    auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
    if(it != m_Layers.end()) {
        m_LayersToRemove.push_back(layer);
        m_LayerInsertIndex--;
    }
}

void LayerStack::popOverlay(Layer *overlay) {
    auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
    if(it != m_Layers.end()) {
        m_LayersToRemove.push_back(overlay);
    }
}

void LayerStack::flush() {
    for (Layer* layer : m_LayersToRemove) {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (it != m_Layers.end()) {
            if (std::distance(m_Layers.begin(), it) < m_LayerInsertIndex) {
                --m_LayerInsertIndex;
            }
            m_Layers.erase(it);
        }
    }
    m_LayersToRemove.clear();

    for (Layer* layer : m_LayersToPushFront) {
        m_Layers.insert(m_Layers.begin() + m_LayerInsertIndex, layer);
        ++m_LayerInsertIndex;
    }
    m_LayersToPushFront.clear();

    for (Layer* overlay : m_LayersToPushBack) {
        m_Layers.push_back(overlay);
    }
    m_LayersToPushBack.clear();
}