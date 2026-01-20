#pragma once
#include "PhysicsSystem.h"
#include "../core/API.h"
#include "../core/Layer.h"
#include "ZeusEngineCore/scripting/CompRegistry.h"
#include "ZeusEngineCore/scripting/SystemManager.h"
#include "ZeusEngineCore/engine/Renderer.h"

struct GLFWwindow;
namespace ZEN {
    class Scene;
    class Renderer;
    class ModelImporter;
    class AssetLibrary;
    class RenderSystem;
    class CameraSystem;
    class EventDispatcher;
    class ZEN_API ZEngine {
    public:
        explicit ZEngine();
        void init();
        ~ZEngine();
        void setAspectRatio(float aspectRatio);
        Renderer& getRenderer(){ return *m_Renderer; }
        ModelImporter& getModelImporter(){return *m_ModelImporter;}
        RenderSystem& getRenderSystem(){return *m_RenderSystem;}
        CameraSystem& getCameraSystem(){return *m_CameraSystem;}
        SystemManager& getSystemManager() {return *m_SystemManager;}
        CompRegistry& getCompRegistry(){return *m_CompRegistry;}
        PhysicsSystem& getPhysicsSystem(){return *m_PhysicsSystem;}
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
        PhysicsSystem* m_PhysicsSystem{};

        std::unique_ptr<SystemManager> m_SystemManager{};
        std::unique_ptr<CompRegistry> m_CompRegistry{};

        eRendererAPI m_API{};

    };
}
