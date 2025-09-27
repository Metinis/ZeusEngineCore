
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
    struct MouseMoveEvent;

    class Scene;

    class CameraSystem {
    public:
        explicit CameraSystem(Scene* scene);
        void onUpdate(float deltaTime);
    private:
        void onResize(const SceneViewResizeEvent& e);
        void onKeyPressed(const KeyPressedEvent& e);
        void onKeyRepeat(const KeyRepeatEvent& e);
        void onKeyReleased(const KeyReleaseEvent& e);

        void onMouseButtonPressed(const MouseButtonPressEvent& e);
        void onMouseButtonReleased(const MouseButtonReleaseEvent& e);
        void onMouseMove(const MouseMoveEvent& e);

        void onPanelFocusEvent(const PanelFocusEvent& e);

        float m_Width{};
        float m_Height{};
        float m_MoveSpeed{5.0f};
        double m_CursorPosLastX{};
        double m_CursorPosLastY{};
        double m_CursorPosX{};
        double m_CursorPosY{};
        bool m_Resized{};
        bool m_PanelSelected{};
        bool m_CursorLocked{};

        Scene* m_Scene{};

        std::unordered_set<int> m_KeysDown{};
        std::unordered_set<int> m_MouseButtonsDown{};
    };
}
