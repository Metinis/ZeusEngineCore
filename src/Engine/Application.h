#pragma once
#include <memory>
#include "ZeusEngineCore/Window.h"
#include "LayerStack.h"

namespace ZEN {

    class Application {
    public:
        Application();
        ~Application();

        void pushLayer(Layer* layer);
        void pushOverlay(Layer* layer);

        void close();

        void run();

    private:
        std::unique_ptr<Window> m_Window{};
        std::unique_ptr<LayerStack> m_LayerStack{};
        bool m_Running { false };

    };

}
