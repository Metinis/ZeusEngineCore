#pragma once
#include "Application.h"
#include "ZeusEngineCore/engine/ZEngine.h"
#include "ZeusEngineCore/core/Window.h"
#include "../../src/Engine/LayerStack.h"
#include "../../src/ImGUILayers/ImGUILayer.h"
#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#define USE_VULKAN

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
#ifdef USE_VULKAN
        std::unique_ptr<VKRenderer> m_VKRenderer{};
        eRendererAPI m_API{ Vulkan };
#endif
#ifdef USE_OPENGL
        eRendererAPI m_API{ OpenGL };
#endif
        std::string m_ResourceRoot{};
        bool m_Running { false };
        bool m_IsPlaying { false };
    };

}
