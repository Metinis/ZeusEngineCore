#pragma once
#include <functional>
#include <memory>
#include "API.h"
#include "ZeusEngineCore/Renderer.h"
struct GLFWwindow;
namespace ZEN {

    class Scene;
    class Renderer;
    class ModelImporter;
    class ModelLibrary;
    class RenderSystem;
    class CameraSystem;
    class ZEngine {
    public:
        explicit ZEngine(eRendererAPI api, GLFWwindow* nativeWindow, const std::string& resourceRoot);
        ~ZEngine();
        void onUpdate(float deltaTime);
        void onRender(const std::function<void()>& uiRender = nullptr);
        Renderer& getRenderer(){return *m_Renderer;}
        ModelImporter& getModelImporter(){return *m_ModelImporter;}
        ModelLibrary& getModelLibrary(){return *m_ModelLibrary;}
        RenderSystem& getRenderSystem(){return *m_RenderSystem;}
        CameraSystem& getCameraSystem(){return *m_CameraSystem;}
        Scene& getScene(){return *m_Scene;}
    private:
        //Core
        std::unique_ptr<Scene> m_Scene{};
        std::unique_ptr<Renderer> m_Renderer{};

        //Libraries/Loaders
        std::unique_ptr<ModelImporter> m_ModelImporter{};
        std::unique_ptr<ModelLibrary> m_ModelLibrary{};

        //Systems
        std::unique_ptr<RenderSystem> m_RenderSystem{};
        std::unique_ptr<CameraSystem> m_CameraSystem{};

        eRendererAPI m_API{};

    };
}
