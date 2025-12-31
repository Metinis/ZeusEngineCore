#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/Components.h"
#include "ZeusEngineCore/Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <ZeusEngineCore/Application.h>

#include "ZeusEngineCore/InputEvents.h"
#include "GLFW/glfw3.h"

using namespace ZEN;

CameraSystem::CameraSystem() : m_Scene(&Application::get().getEngine()->getScene()){


}

void CameraSystem::onUpdate(float deltaTime) {
    if (m_PlayMode) {
        return;
    }
    auto view = m_Scene->getEntities<CameraComp>();

    for (auto entity : view) {
        auto &camera = entity.getComponent<CameraComp>();

        //update position if has transform
        if (auto *transform = entity.tryGetComponent<TransformComp>()) {
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
                glm::vec3 toMove = glm::normalize(moveVec) * deltaTime * m_MoveSpeed;
                if(m_KeysDown.contains(GLFW_KEY_LEFT_CONTROL)) {
                    toMove /= 4;
                }
                transform->localPosition += toMove;
            }
            if(m_CursorLocked) {
                constexpr float sensitivity = 0.1f;
                double xOffset = m_CursorPosLastX - m_CursorPosX;
                double yOffset = m_CursorPosLastY - m_CursorPosY;

                transform->localRotation.y += xOffset * sensitivity;

                float rotationX = transform->localRotation.x + yOffset * sensitivity;
                rotationX = glm::clamp(rotationX, -89.0f, 89.0f);
                transform->localRotation.x = rotationX;

                m_CursorPosLastX = m_CursorPosX;
                m_CursorPosLastY = m_CursorPosY;
            }
        }
        //if(m_Resized) {
            camera.aspect = m_AspectRatio;
            camera.projection = glm::perspective(camera.fov, camera.aspect, camera.near, camera.far);
            //m_Resized = false;
        //}

    }

}

void CameraSystem::onEvent(Event &event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) {return onKeyPressed(e); });
    dispatcher.dispatch<KeyReleasedEvent>([this](KeyReleasedEvent& e) {return onKeyReleased(e); });
    dispatcher.dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e) {return onMouseButtonPressed(e); });
    dispatcher.dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& e) {return onMouseButtonReleased(e); });
    dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) {return onMouseMove(e); });
    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayMode(e); });

}

bool CameraSystem::onPlayMode(RunPlayModeEvent &e) {
    m_PlayMode = e.getPlaying();
}

bool CameraSystem::onKeyPressed(const KeyPressedEvent &e) {
    m_KeysDown.insert(e.getKeyCode());
    return true;
}

bool CameraSystem::onKeyRepeat(const KeyPressedEvent &e) {
    return true;
}

bool CameraSystem::onKeyReleased(const KeyReleasedEvent &e) {
    m_KeysDown.erase(e.getKeyCode());
    return true;
}

bool CameraSystem::onMouseButtonPressed(const MouseButtonPressedEvent &e) {
    if(e.getKeyCode() == GLFW_MOUSE_BUTTON_RIGHT) {
        Application::get().getWindow()->setCursorLock(true, m_CursorPosX, m_CursorPosY);
        m_CursorLocked = true;
        m_CursorPosLastX = m_CursorPosX;
        m_CursorPosLastY = m_CursorPosY;
    }
    return true;
}

bool CameraSystem::onMouseButtonReleased(const MouseButtonReleasedEvent &e) {
    if(e.getKeyCode() == GLFW_MOUSE_BUTTON_RIGHT) {
        Application::get().getWindow()->setCursorLock(false, m_CursorPosX, m_CursorPosY);
        m_CursorLocked = false;
    }
    return true;
}

bool CameraSystem::onMouseMove(const MouseMovedEvent &e) {
    m_CursorPosX = e.getXPos();
    m_CursorPosY = e.getYPos();
    return true;
}
