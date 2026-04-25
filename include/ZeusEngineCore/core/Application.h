#pragma once
#include "Application.h"
#include "ZeusEngineCore/core/Window.h"
#include "../../src/Engine/LayerStack.h"
#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#define USE_VULKAN

namespace ZEN {
    class ImGUILayerVulkan;
    class WindowResizeEvent;
    class RunPlayModeEvent;
    class CompRegistry;
    class ZeusPhysicsSystem;
    class CameraSystem;
    class RenderSystem;
    class ModelImporter;
    class Renderer;
    class Scene;
    class SystemManager;

    struct EngineContext {
        //Core
        Scene* scene{};
        std::unique_ptr<VKRenderer> vkRenderer{};
        std::unique_ptr<Window> window{};

        //Libraries/Loaders
        std::unique_ptr<ModelImporter> modelImporter{};

        //Systems
        CameraSystem* cameraSystem{};
        ZeusPhysicsSystem* physicsSystem{};

        std::unique_ptr<SystemManager> systemManager{};
        std::unique_ptr<CompRegistry> compRegistry{};
    };

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

        Window* getWindow() {return m_Ctx.window.get();}
        static Application& get() { return *s_Instance; }
        std::string getResourceRoot() { return m_ResourceRoot; }
        void close();
        void run();
    private:
        std::unique_ptr<LayerStack> m_LayerStack{};
    protected:
        static Application* s_Instance;

        std::unique_ptr<ImGUILayerVulkan> m_ImGUILayer{};
        EngineContext m_Ctx;

        std::string m_ResourceRoot{};
        bool m_Running { false };
        bool m_IsPlaying { false };
    };

}
