#pragma once

namespace ZEN {
    struct KeyPressedEvent {
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
    };
    struct MouseMoveEvent {
        double xPos;
        double yPos;
    };
}