#pragma once
#include "API.h"
#include "Layer.h"
#include "ZeusEngineCore/Renderer.h"

struct GLFWwindow;
namespace ZEN {
    class Scene;
    class Renderer;
    class ModelImporter;
    class AssetLibrary;
    class RenderSystem;
    class CameraSystem;
    class EventDispatcher;
    class ZEngine {
    public:
        explicit ZEngine(eRendererAPI api, GLFWwindow* nativeWindow, const std::string& resourceRoot);
        ~ZEngine();
        void setAspectRatio(float aspectRatio);
        Renderer& getRenderer(){return *m_Renderer;}
        ModelImporter& getModelImporter(){return *m_ModelImporter;}
        RenderSystem& getRenderSystem(){return *m_RenderSystem;}
        CameraSystem& getCameraSystem(){return *m_CameraSystem;}
        Scene& getScene(){return *m_Scene;}
    private:
        //Core
        Scene* m_Scene{};
        Renderer* m_Renderer{};

        //Libraries/Loaders
        std::unique_ptr<ModelImporter> m_ModelImporter{};

        //Systems
        RenderSystem* m_RenderSystem{};
        CameraSystem* m_CameraSystem{};

        eRendererAPI m_API{};

    };
}
