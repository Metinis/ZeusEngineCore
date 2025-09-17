#pragma once

namespace ZEN {
    struct KeyPressedEvent {
        int key;    // GLFW_KEY_*
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
}