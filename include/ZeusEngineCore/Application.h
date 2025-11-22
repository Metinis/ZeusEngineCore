#pragma once
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
        static Application& get() { return *s_Instance; }

        void close();

        void run();

    protected:
        static Application* s_Instance;

        std::unique_ptr<Window> m_Window{};
        std::unique_ptr<ImGUILayer> m_ImGUILayer{};
        std::unique_ptr<LayerStack> m_LayerStack{};
        std::unique_ptr<ZEngine> m_Engine{};

        std::string m_ResourceRoot{};
        eRendererAPI m_API{OpenGL};
        bool m_Running { false };

    };

}
