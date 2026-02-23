#pragma once
#include "ZeusEngineCore/core/API.h"

namespace ZEN {
    struct CollisionEvent;
    class Scene;

    class ZEN_API ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void onUpdate(float dt) = 0;
        virtual void onCollisionEnter(const CollisionEvent& e) = 0;
        virtual void onCollisionStay(const CollisionEvent& e) = 0;
        virtual void onCollisionExit(const CollisionEvent& e) = 0;
        virtual void onLoad(Scene* scene) {
            m_Scene = scene;
        };
        virtual void onUnload() = 0;
    protected:
        Scene* m_Scene{};
    };
}