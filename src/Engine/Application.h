#pragma once
#include <memory>
#include "LayerStack.h"

namespace ZEN {

    class Application {
    public:
        Application();
        ~Application();

        void pushLayer(Layer* layer);
        void pushOverlay(Layer* layer);

        void run();

    private:
        std::unique_ptr<LayerStack> m_LayerStack{};

    };

}
