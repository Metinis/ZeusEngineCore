#pragma once
#include "Layer.h"
#include "ZeusEngineCore/InputEvents.h"

namespace ZEN {
    /*struct SceneViewResizeEvent;
    struct KeyPressedEvent;
    struct KeyRepeatEvent;
    struct KeyReleaseEvent;
    struct MouseButtonPressEvent;
    struct MouseButtonReleaseEvent;
    struct PanelFocusEvent;
    struct MouseMoveEvent;*/

    class Scene;
    class EventDispatcher;

    class CameraSystem : public Layer {
    public:
        explicit CameraSystem(Scene* scene);
        void onUpdate(float deltaTime) override;
        void onEvent(Event& event) override;
        void setSize(int width, int height);
    private:
        bool onViewportResize(const ViewportResizeEvent& e);
        bool onKeyPressed(const KeyPressedEvent& e);
        bool onKeyRepeat(const KeyPressedEvent& e);
        bool onKeyReleased(const KeyReleasedEvent& e);

        bool onMouseButtonPressed(const MouseButtonPressedEvent& e);
        bool onMouseButtonReleased(const MouseButtonReleasedEvent& e);
        bool onMouseMove(const MouseMovedEvent& e);

        //void onPanelFocusEvent(const PanelFocusedEvent& e);

        float m_Width{};
        float m_Height{};
        float m_MoveSpeed{5.0f};
        double m_CursorPosLastX{};
        double m_CursorPosLastY{};
        double m_CursorPosX{};
        double m_CursorPosY{};
        bool m_Resized{true};
        bool m_PanelSelected{};
        bool m_CursorLocked{};

        Scene* m_Scene{};

        std::unordered_set<int> m_KeysDown{};
        std::unordered_set<int> m_MouseButtonsDown{};
    };
}
