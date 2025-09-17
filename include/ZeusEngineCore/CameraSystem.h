
#pragma once
#include <entt.hpp>

namespace ZEN {
    struct SceneViewResizeEvent;
    class CameraSystem {
    public:
        explicit CameraSystem(entt::dispatcher& dispatcher);
        void onUpdate(entt::registry& registry);
    private:
        void onResize(const SceneViewResizeEvent& e);

        float m_Width{};
        float m_Height{};
        bool m_Resized{};
    };
}
