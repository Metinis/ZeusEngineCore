#pragma once
#include "ZeusEngineCore/core/API.h"

namespace ZEN {
    class Scene;

    class ZEN_API ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void onUpdate(float dt) = 0;
        virtual void onLoad(Scene* scene) {
            m_Scene = scene;
        };
        virtual void onUnload() = 0;
    protected:
        Scene* m_Scene{};
    };
}