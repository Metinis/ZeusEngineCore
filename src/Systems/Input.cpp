#include "ZeusEngineCore/input/Input.h"

#include "GLFW/glfw3.h"
#include "ZeusEngineCore/core/Application.h"

using namespace ZEN;

Input* Input::s_Instance = new Input();

bool Input::isKeyPressed(int keycode) {
    auto window = Application::get().getWindow()->getNativeWindow();
    auto state = glfwGetKey(window, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isMouseButtonPressed(int keycode) {
    auto window = Application::get().getWindow()->getNativeWindow();
    auto state = glfwGetMouseButton(window, keycode);
    return state == GLFW_PRESS;
}

std::pair<float, float> Input::getMousePos() {
    auto window = Application::get().getWindow()->getNativeWindow();
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    return {xPos, yPos};
}

float Input::getMouseX() {
    return getMousePos().first;
}

float Input::getMouseY() {
    return getMousePos().second;
}
