#pragma once

#include <vector>
#include "ZeusEngineCore/Layer.h"

namespace ZEN {
    class LayerStack {
    public:
        LayerStack();
        ~LayerStack();

        void pushLayer(Layer* layer);
        void pushOverlay(Layer* overlay);
        void popLayer(Layer* layer);
        void popOverlay(Layer* overlay);

    private:

        std::vector<Layer*> m_Layers{};
        std::vector<Layer*>::iterator m_LayerInsert{};
    };
}