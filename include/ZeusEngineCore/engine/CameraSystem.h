#pragma once
#include "ZeusEngineCore/core/Layer.h"
#include "ZeusEngineCore/core/InputEvents.h"

namespace ZEN {
    class Scene;
    class EventDispatcher;

    class CameraSystem : public Layer {
    public:
        explicit CameraSystem();
        void onUpdate(float deltaTime) override;
        void onEvent(Event& event) override;
        void setAspectRatio(float aspectRatio) { m_Resized = true; m_AspectRatio = aspectRatio; }
        void setUseMainCamera(bool useMainCamera) { m_UseMainCamera = useMainCamera; }
        bool getUseMainCamera() { return m_UseMainCamera; }
    private:
        bool onPlayMode(RunPlayModeEvent& e);
        bool onKeyPressed(const KeyPressedEvent& e);
        bool onKeyRepeat(const KeyPressedEvent& e);
        bool onKeyReleased(const KeyReleasedEvent& e);

        bool onMouseButtonPressed(const MouseButtonPressedEvent& e);
        bool onMouseButtonReleased(const MouseButtonReleasedEvent& e);
        bool onMouseMove(const MouseMovedEvent& e);

        float m_AspectRatio{};
        float m_MoveSpeed{5.0f};
        double m_CursorPosLastX{};
        double m_CursorPosLastY{};
        double m_CursorPosX{};
        double m_CursorPosY{};
        bool m_PlayMode{false};
        bool m_UseMainCamera{false};
        bool m_Resized{true};
        bool m_PanelSelected{};
        bool m_CursorLocked{};

        Scene* m_Scene{};

        std::unordered_set<int> m_KeysDown{};
        std::unordered_set<int> m_MouseButtonsDown{};
    };
}
