#pragma once
#include "GLFW/glfw3.h"

namespace ZEN
{
    namespace Mouse
    {
        enum Button : int
        {
            Button1      = GLFW_MOUSE_BUTTON_1,
            Button2      = GLFW_MOUSE_BUTTON_2,
            Button3      = GLFW_MOUSE_BUTTON_3,
            Button4      = GLFW_MOUSE_BUTTON_4,
            Button5      = GLFW_MOUSE_BUTTON_5,
            Button6      = GLFW_MOUSE_BUTTON_6,
            Button7      = GLFW_MOUSE_BUTTON_7,
            Button8      = GLFW_MOUSE_BUTTON_8,

            ButtonLast   = GLFW_MOUSE_BUTTON_LAST,
            ButtonLeft   = GLFW_MOUSE_BUTTON_LEFT,
            ButtonRight  = GLFW_MOUSE_BUTTON_RIGHT,
            ButtonMiddle = GLFW_MOUSE_BUTTON_MIDDLE
        };
    }
}
