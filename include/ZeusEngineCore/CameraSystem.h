
#pragma once
#include <entt.hpp>

namespace ZEN {
    class CameraSystem {
    public:
        static void onUpdate(entt::registry& registry, float windowWidth, float windowHeight);
    };
}
