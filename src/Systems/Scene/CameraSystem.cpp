#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/Components.h"
#include <glm/gtc/matrix_transform.hpp>
#include "ZeusEngineCore/InputEvents.h"
#include <iostream>

#include "GLFW/glfw3.h"

using namespace ZEN;

CameraSystem::CameraSystem(entt::dispatcher &dispatcher) : m_Dispatcher(dispatcher){
    dispatcher.sink<SceneViewResizeEvent>().connect<&CameraSystem::onResize>(*this);
    dispatcher.sink<KeyPressedEvent>().connect<&CameraSystem::onKeyPressed>(*this);
    dispatcher.sink<KeyRepeatEvent>().connect<&CameraSystem::onKeyRepeat>(*this);
    dispatcher.sink<KeyReleaseEvent>().connect<&CameraSystem::onKeyReleased>(*this);
    dispatcher.sink<MouseButtonPressEvent>().connect<&CameraSystem::onMouseButtonPressed>(*this);
    dispatcher.sink<MouseButtonReleaseEvent>().connect<&CameraSystem::onMouseButtonReleased>(*this);
    dispatcher.sink<PanelFocusEvent>().connect<&CameraSystem::onPanelFocusEvent>(*this);
    dispatcher.sink<MouseMoveEvent>().connect<&CameraSystem::onMouseMove>(*this);
}

void CameraSystem::onUpdate(entt::registry &registry, float deltaTime) {
    auto view = registry.view<CameraComp>();

    for (auto entity : view) {
        auto &camera = view.get<CameraComp>(entity);

        //update position if has transform
        if (auto *transform = registry.try_get<TransformComp>(entity)) {
            glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 front = transform->getFront();
            glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
            glm::vec3 right = glm::normalize(glm::cross(flatFront, worldUp));
            glm::vec3 up = worldUp;

            glm::vec3 moveVec = {};
            if (m_KeysDown.contains(GLFW_KEY_W)) moveVec += front;
            if (m_KeysDown.contains(GLFW_KEY_S)) moveVec -= front;
            if (m_KeysDown.contains(GLFW_KEY_A)) moveVec -= right;
            if (m_KeysDown.contains(GLFW_KEY_D)) moveVec += right;
            if (m_KeysDown.contains(GLFW_KEY_SPACE)) moveVec += up;
            if (m_KeysDown.contains(GLFW_KEY_LEFT_SHIFT)) moveVec -= up;

            if (glm::length(moveVec) > 0.0f) {
                transform->position += glm::normalize(moveVec) * deltaTime * m_MoveSpeed;
            }
            if(m_CursorLocked) {
                constexpr float sensitivity = 0.1f;
                double xOffset = m_CursorPosLastX - m_CursorPosX;
                double yOffset = m_CursorPosLastY - m_CursorPosY;

                transform->rotation.y += xOffset * sensitivity;
                transform->rotation.x += yOffset * sensitivity;

                m_CursorPosLastX = m_CursorPosX;
                m_CursorPosLastY = m_CursorPosY;
            }
        }

        //update perspective matrix if resized
        if(m_Resized) {
            camera.aspect = (float)m_Width / m_Height;
            std::cout<<"Aspect: "<< m_Width/m_Height<<"\n";
            camera.projection = glm::perspective(camera.fov, camera.aspect, camera.near, camera.far);
            m_Resized = false;
        }

    }

}

void CameraSystem::onResize(const SceneViewResizeEvent &e) {
    std::cout<<"Camera resized! "<<e.width<<"x "<<e.height<<"y \n";
    m_Width = e.width;
    m_Height = e.height;
    m_Resized = true;
}

void CameraSystem::onKeyPressed(const KeyPressedEvent &e) {
    m_KeysDown.insert(e.key);
}

void CameraSystem::onKeyRepeat(const KeyRepeatEvent &e) {

}

void CameraSystem::onKeyReleased(const KeyReleaseEvent &e) {
    m_KeysDown.erase(e.key);
}

void CameraSystem::onMouseButtonPressed(const MouseButtonPressEvent &e) {
    //m_MouseButtonsDown.insert(e.button);
    if(e.button == GLFW_MOUSE_BUTTON_RIGHT && m_PanelSelected) {
        m_Dispatcher.trigger<CursorLockEvent>(CursorLockEvent{true});
        m_CursorLocked = true;
    }
}

void CameraSystem::onMouseButtonReleased(const MouseButtonReleaseEvent &e) {
    //m_MouseButtonsDown.erase(e.button);
    if(e.button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_Dispatcher.trigger<CursorLockEvent>(CursorLockEvent{false});
        m_CursorLocked = false;
    }
}

void CameraSystem::onMouseMove(const MouseMoveEvent &e) {
    m_CursorPosX = e.xPos;
    m_CursorPosY = e.yPos;
    if(!m_CursorLocked) {
        m_CursorPosLastX = m_CursorPosX;
        m_CursorPosLastY = m_CursorPosY;
    }
}

void CameraSystem::onPanelFocusEvent(const PanelFocusEvent &e) {
    m_PanelSelected = e.panel == "Scene View";
}

