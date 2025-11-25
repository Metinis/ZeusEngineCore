#pragma once
//#include "ZeusEngineCore/Entity.h"
#include "ZeusEngineCore/Event.h"

namespace ZEN {
    class Entity;
    struct MaterialComp;
    /*struct KeyPressedEvent {
        int key;
        int scancode;
        int mods;
    };
    struct KeyRepeatEvent {
        int key;
        int scancode;
        int mods;
    };
    struct KeyReleaseEvent {
        int key;
        int scancode;
        int mods;
    };
    struct WindowResizeEvent {
        int width;
        int height;
    };
    struct SceneViewResizeEvent {
        float width;
        float height;
    };
    struct MouseButtonPressEvent {
        int button;
        int mods;
    };
    struct MouseButtonReleaseEvent {
        int button;
        int mods;
    };
    struct MouseButtonRepeatEvent {
        int button;
        int mods;
    };
    struct PanelFocusEvent {
        std::string panel;
    };
    struct CursorLockEvent {
        bool lock;
        double xPos;
        double yPos;
    };
    struct MouseMoveEvent {
        double xPos;
        double yPos;
    };
    struct SelectEntityEvent {
        Entity entity;
    };
    struct SelectMaterialEvent {
        std::string materialName;
    };
    struct RemoveMeshEvent {
        std::string meshName;
    };
    struct RemoveMaterialEvent {
        std::string materialName;
    };
    struct RemoveTextureEvent {
        std::string textureName;
    };
    struct RemoveMeshCompEvent {
        Entity entity;
    };
    struct RemoveMeshDrawableEvent {
        Entity entity;
    };
    struct ToggleDrawNormalsEvent {

    };
    struct ToggleEditorEvent {

    };*/

    //-----------------------Key Events---------------------------
    class KeyEvent : public Event {
    public:
        int getKeyCode() const { return m_KeyCode; }
    protected:
        KeyEvent(int keycode) : m_KeyCode(keycode) {}
        int m_KeyCode{};
    };
    class KeyPressedEvent : public KeyEvent {
    public:
        KeyPressedEvent(int keycode, bool repeat) : KeyEvent(keycode),
        m_Repeat(repeat) {}

        bool getRepeat() const { return m_Repeat; }
        std::string toString() const override {
            return std::format("Key Pressed Event: {}, repeat = {}", m_KeyCode, m_Repeat);
        }
        EVENT_CLASS_TYPE(KeyPressed)
    private:
        bool m_Repeat{false};
    };
    class KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

        std::string toString() const override {
            return std::format("Key Released Event: {}", m_KeyCode);
        }
        EVENT_CLASS_TYPE(KeyReleased)
    };
    //------------------------------------------------------------

    //-----------------------Mouse Events-------------------------
    class MouseButtonEvent : public Event {
    public:
        MouseButtonEvent(int button) : m_Button(button) {}
        int getKeyCode() const { return m_Button; }
    protected:
        int m_Button{};
    };

    class MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        MouseButtonPressedEvent(int button, bool repeat) : MouseButtonEvent(button), m_Repeat(repeat) {}

        std::string toString() const override {
            return std::format("Mouse Button Pressed Event: {}, (repeat: {})", m_Button, m_Repeat);
        }
        EVENT_CLASS_TYPE(MouseButtonPressed)
    private:
        bool m_Repeat{false};
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

        std::string toString() const override {
            return std::format("Mouse Button Released Event: {}", m_Button);
        }
        EVENT_CLASS_TYPE(MouseButtonReleased)
    };

    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(int xPos, int yPos) : m_XPos(xPos), m_YPos(yPos) {}

        int getXPos() const { return m_XPos; }
        int getYPos() const { return m_YPos; }

        std::string toString() const override {
            return std::format("Mouse Moved Event: x = {} y = {} ", m_XPos, m_YPos);
        }

        EVENT_CLASS_TYPE(MouseMoved)
    private:
        int m_XPos{};
        int m_YPos{};
    };
    //------------------------------------------------------------

    //-----------------------Window Events------------------------
    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(int width, int height) : m_Width(width), m_Height(height) {}
        int getWidth() const { return m_Width; }
        int getHeight() const { return m_Height; }
        std::string toString() const override {
            return std::format("Window Resize Event: {} {}", m_Width, m_Height);
        }

        EVENT_CLASS_TYPE(WindowResize)
    private:
        int m_Width{};
        int m_Height{};
    };
    class ViewportResizeEvent : public Event {
    public:
        ViewportResizeEvent(int width, int height) : m_Width(width), m_Height(height) {}
        int getWidth() const { return m_Width; }
        int getHeight() const { return m_Height; }
        std::string toString() const override {
            return std::format("Viewport Resize Event: {} {}", m_Width, m_Height);
        }

        EVENT_CLASS_TYPE(ViewportResize)
    private:
        int m_Width{};
        int m_Height{};
    };
    //------------------------------------------------------------

    //-----------------------Resource Events----------------------
    class RemoveResourceEvent : public Event {
    public:
        RemoveResourceEvent(std::string resourceName) : m_ResourceName(resourceName) {}
        std::string toString() const override {
            return std::format("Resource Remove Event: {}", m_ResourceName);
        }

        EVENT_CLASS_TYPE(RemoveResource)
    private:
        std::string m_ResourceName{};
    };
    //------------------------------------------------------------

}