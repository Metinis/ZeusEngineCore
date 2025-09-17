
#pragma once
#include <entt.hpp>
#include <glm/vec3.hpp>

namespace ZEN {
    struct SceneViewResizeEvent;
    struct KeyPressedEvent;
    struct KeyRepeatEvent;
    struct KeyReleaseEvent;
    struct MouseButtonPressEvent;
    struct MouseButtonReleaseEvent;
    struct PanelFocusEvent;
    class CameraSystem {
    public:
        explicit CameraSystem(entt::dispatcher& dispatcher);
        void onUpdate(entt::registry &registry, float deltaTime);
    private:
        void onResize(const SceneViewResizeEvent& e);
        void onKeyPressed(const KeyPressedEvent& e);
        void onKeyRepeat(const KeyRepeatEvent& e);
        void onKeyReleased(const KeyReleaseEvent& e);

        void onMouseButtonPressed(const MouseButtonPressEvent& e);
        void onMouseButtonReleased(const MouseButtonReleaseEvent& e);

        void onPanelFocusEvent(const PanelFocusEvent& e);

        float m_Width{};
        float m_Height{};
        float m_MoveSpeed{5.0f};
        bool m_Resized{};
        bool m_PanelSelected{};
        bool m_CursorLocked{};

        entt::dispatcher& m_Dispatcher;

        std::unordered_set<int> m_KeysDown{};
        std::unordered_set<int> m_MouseButtonsDown{};
    };
}
