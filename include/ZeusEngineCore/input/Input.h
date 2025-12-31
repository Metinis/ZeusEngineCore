#pragma once

namespace ZEN {
    class Input {
    public:
        static bool isKeyPressed(int keycode);
        static bool isMouseButtonPressed(int keycode);
        static std::pair<float, float> getMousePos();
        static float getMouseX();
        static float getMouseY();
    private:
        static Input* s_Instance;
    };
}
