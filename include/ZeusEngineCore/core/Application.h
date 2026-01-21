#pragma once
#include "Application.h"
#include "ZeusEngineCore/engine/ZEngine.h"
#include "ZeusEngineCore/core/Window.h"
#include "../../src/Engine/LayerStack.h"
#include "../../src/ImGUILayers/ImGUILayer.h"

namespace ZEN {

    class ZEN_API Application {
    public:
        Application();
        virtual ~Application();

        void init();
        void pushLayer(Layer* layer);
        void pushOverlay(Layer* layer);
        void popLayer(Layer* layer);
        void popOverlay(Layer* layer);
        void callEvent(Event& event);

        bool onPlayMode(const RunPlayModeEvent &event);

        bool onWindowResize(const WindowResizeEvent& event);

        Window* getWindow() const { return m_Window.get(); }
        static Application& get() { return *s_Instance; }
        eRendererAPI getRendererAPI() const { return m_API; }
        ZEngine* getEngine() const { return m_Engine.get(); }
        std::string getResourceRoot() { return m_ResourceRoot; }

        void close();

        void run();
    private:
        std::unique_ptr<LayerStack> m_LayerStack{};
    protected:
        static Application* s_Instance;

        std::unique_ptr<Window> m_Window{};
        std::unique_ptr<ImGUILayer> m_ImGUILayer{};
        
        std::unique_ptr<ZEngine> m_Engine{};

        std::string m_ResourceRoot{};
        eRendererAPI m_API{ OpenGL };
        bool m_Running { false };
        bool m_IsPlaying { false };

    };

}
