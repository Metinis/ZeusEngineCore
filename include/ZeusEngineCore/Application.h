#pragma once
#include <memory>
#include "ZeusEngineCore/ZEngine.h"
#include "ZeusEngineCore/Window.h"
#include "../../src/Engine/LayerStack.h"
#include "../../src/ImGUILayers/ImGUILayer.h"

namespace ZEN {

    class Application {
    public:
        Application();
        virtual ~Application();

        void init();
        void pushLayer(Layer* layer);
        void pushOverlay(Layer* layer);

        void close();

        void run();

    protected:
        std::unique_ptr<Window> m_Window{};
        std::unique_ptr<ImGUILayer> m_ImGUILayer{};
        std::unique_ptr<LayerStack> m_LayerStack{};
        std::unique_ptr<ZEngine> m_Engine{};

        std::string m_ResourceRoot{};
        eRendererAPI m_API{OpenGL};
        bool m_Running { false };

    };

}
