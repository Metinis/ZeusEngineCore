#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/Components.h"
#include <glm/gtc/matrix_transform.hpp>
#include "ZeusEngineCore/InputEvents.h"
#include <iostream>

using namespace ZEN;

CameraSystem::CameraSystem(entt::dispatcher &dispatcher) {
    dispatcher.sink<SceneViewResizeEvent>().connect<&CameraSystem::onResize>(*this);
}

void CameraSystem::onUpdate(entt::registry &registry) {
    if(!m_Resized)
        return;

    auto view = registry.view<CameraComp>();

    for (auto entity : view) {
        auto &camera = view.get<CameraComp>(entity);

        camera.aspect = (float)m_Width / m_Height;
        std::cout<<"Aspect: "<< m_Width/m_Height<<"\n";
        //camera.fov = glm::radians(45.0f);  // vertical fov


        camera.projection = glm::perspective(camera.fov, camera.aspect, camera.near, camera.far);
    }
    m_Resized = false;
}

void CameraSystem::onResize(const SceneViewResizeEvent &e) {
    std::cout<<"Camera resized! "<<e.width<<"x "<<e.height<<"y \n";
    m_Width = e.width;
    m_Height = e.height;
    m_Resized = true;
}
