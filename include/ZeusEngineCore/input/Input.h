#pragma once
#include "ZeusEngineCore/core/API.h"

namespace ZEN {
    class ZEN_API Input {
    public:
        static bool isKeyPressed(int keycode);
        static bool isMouseButtonPressed(int keycode);
        static std::pair<float, float> getMousePos();
        static void setMouseLock(bool lock);
        static float getMouseX();
        static float getMouseY();
    private:
        static Input* s_Instance;
    };
}
