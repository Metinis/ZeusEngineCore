#include "ZeusEngineCore/engine/CameraSystem.h"
#include "ZeusEngineCore/engine/Components.h"
#include "ZeusEngineCore/engine/Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <ZeusEngineCore/core/Application.h>
#include "ZeusEngineCore/core/InputEvents.h"
#include "ZeusEngineCore/input/KeyCodes.h"
#include "ZeusEngineCore/input/MouseCodes.h"

using namespace ZEN;

CameraSystem::CameraSystem() : m_Scene(&Application::get().getEngine()->getScene()){


}

void CameraSystem::onUpdate(float deltaTime) {
    if (m_PlayMode || m_UseMainCamera) {
        auto gameCamView = m_Scene->getEntities<CameraComp>();

        for (auto entity : gameCamView) {
            auto &camera = entity.getComponent<CameraComp>();
            camera.aspect = m_AspectRatio;
            camera.projection = glm::perspective(camera.fov, camera.aspect, camera.near, camera.far);
        }
        return;
    }
    auto view = m_Scene->getEntities<SceneCameraComp>();

    for (auto entity : view) {
        auto &camera = entity.getComponent<SceneCameraComp>();

        //update position if has transform
        if (auto *transform = entity.tryGetComponent<TransformComp>()) {
            glm::vec3 front = transform->getFrontWorld();
            glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f,1.0f,0.0f)));
            glm::vec3 up = glm::normalize(glm::cross(right, front));

            glm::vec3 moveVec = {};
            if (m_KeysDown.contains(Key::W)) moveVec += front;
            if (m_KeysDown.contains(Key::S)) moveVec -= front;
            if (m_KeysDown.contains(Key::A)) moveVec -= right;
            if (m_KeysDown.contains(Key::D)) moveVec += right;
            if (m_KeysDown.contains(Key::Space)) moveVec += up;
            if (m_KeysDown.contains(Key::LeftShift)) moveVec -= up;

            if (glm::length(moveVec) > 0.0f) {
                glm::vec3 toMove = glm::normalize(moveVec) * deltaTime * m_MoveSpeed;
                if(m_KeysDown.contains(Key::LeftControl)) {
                    toMove /= 4;
                }
                transform->localPosition += toMove;
            }
            if(m_CursorLocked) {
                constexpr float sensitivity = 0.1f;
                float xOffset = m_CursorPosX - m_CursorPosLastX;
                float yOffset = m_CursorPosY - m_CursorPosLastY;

                xOffset *= sensitivity;
                yOffset *= sensitivity;

                glm::quat yaw = glm::angleAxis(glm::radians(-xOffset), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::quat pitch = glm::angleAxis(glm::radians(-yOffset), right);

                transform->localRotation = glm::normalize(yaw * pitch * transform->localRotation);

                m_CursorPosLastX = m_CursorPosX;
                m_CursorPosLastY = m_CursorPosY;
            }
        }
        camera.aspect = m_AspectRatio;
        camera.projection = glm::perspective(camera.fov, camera.aspect, camera.near, camera.far);
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
    return false;
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
    if(e.getKeyCode() == Mouse::Button::ButtonRight && !m_PlayMode) {
        Application::get().getWindow()->setCursorLock(true, m_CursorPosX, m_CursorPosY);
        m_CursorLocked = true;
        m_CursorPosLastX = m_CursorPosX;
        m_CursorPosLastY = m_CursorPosY;
    }
    return true;
}

bool CameraSystem::onMouseButtonReleased(const MouseButtonReleasedEvent &e) {
    if(e.getKeyCode() == Mouse::Button::ButtonRight && !m_PlayMode) {
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
