
#include "ZeusEngineCore/CameraSystem.h"
#include "ZeusEngineCore/Components.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace ZEN;

void CameraSystem::onUpdate(entt::registry &registry, float windowWidth, float windowHeight) {
    auto view = registry.view<CameraComp>();

    for (auto entity : view) {
        auto &camera = view.get<CameraComp>(entity);

        camera.aspect = windowWidth / windowHeight;

        camera.projection = glm::perspective(camera.fov, camera.aspect, camera.near, camera.far);
    }
}
