#include "ZeusEngineCore/Scene.h"

#include <ZeusEngineCore/Application.h>
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/InputEvents.h>
#include <ZeusEngineCore/AssetLibrary.h>
#include <ZeusEngineCore/ZEngine.h>
#include "ZeusEngineCore/SceneSerializer.h"

using namespace ZEN;

Scene::Scene() {
    Application::get().getEngine()->getSystemManager().loadAllFromDirectory(Project::getActive()->getActiveProjectRoot() +
        "assets/scripts/bin/", this);
}

void Scene::onUpdate(float dt) {
    if (m_PlayMode) {
        Application::get().getEngine()->getSystemManager().updateAll(dt);
    }
}


void Scene::createDefaultScene() {
    auto dirLightEntity = createEntity("Directional Light");
    DirectionalLightComp comp {
        .ambient = {0.1f, 0.1f, 0.1f},
        .isPrimary = true,
    };
    dirLightEntity.addComponent<DirectionalLightComp>(comp);

    auto cameraEntity = createEntity("Scene Camera");
    cameraEntity.addComponent<CameraComp>();

    auto cubeEntity = createEntity("Cube");
    auto assetLibrary = Project::getActive()->getAssetLibrary();
    cubeEntity.addComponent<MeshComp>(AssetHandle<MeshData>(assetLibrary->getCubeID()));

    auto skyboxEntity = createEntity("Skybox");
    skyboxEntity.addComponent<SkyboxComp>();
    skyboxEntity.addComponent<MeshComp>(AssetHandle<MeshData>(assetLibrary->getSkyboxID()));
}

Entity Scene::createEntity(const std::string& name) {
	auto ret = Entity{this, m_Registry.create()};
    ret.addComponent<UUIDComp>();
    ret.addComponent<TransformComp>();
    auto view = getEntities<CameraComp>();
    for (auto entity : view) {
        auto& camera = entity.getComponent<CameraComp>();
        auto& cameraTransform = entity.getComponent<TransformComp>();
        if(camera.isPrimary) {
            ret.getComponent<TransformComp>() = TransformComp{.localPosition =
                cameraTransform.localPosition + cameraTransform.getFront() * 5.0f};
            break;
        }
    }
    TagComp tag {.tag = "Unnamed Entity"};
    if(!name.empty()) {
        tag.tag = name;
    }
    ret.addComponent<TagComp>(tag);
    return ret;
}
Entity Scene::createEntity(const std::string& name, UUID id) {
    auto ret = Entity{this, m_Registry.create()};
    ret.addComponent<UUIDComp>(id);
    ret.addComponent<TransformComp>();
    auto view = getEntities<CameraComp>();
    for (auto entity : view) {
        auto& camera = entity.getComponent<CameraComp>();
        auto& cameraTransform = entity.getComponent<TransformComp>();
        if(camera.isPrimary) {
            ret.getComponent<TransformComp>() = TransformComp{.localPosition =
                cameraTransform.localPosition + cameraTransform.getFront() * 5.0f};
            break;
        }
    }
    TagComp tag {.tag = "Unnamed Entity"};
    if(!name.empty()) {
        tag.tag = name;
    }
    ret.addComponent<TagComp>(tag);
    return ret;
}

Entity Scene::getEntity(UUID id) {
    auto view = getEntities<UUIDComp>();
    for (auto entity : view) {
        if (entity.getComponent<UUIDComp>().uuid == id) {
            return entity;
        }
    }
    return Entity{};
}

void Scene::removeEntity(Entity entity) {
    m_Registry.destroy((entt::entity)entity);
}



void Scene::onEvent(Event &event) {
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayMode(e); });
}

std::vector<Entity> Scene::getEntities(const std::string &name) {
    std::vector<Entity> result;

    for (auto& [entity, comps] : m_RuntimeComponents) {
        if (comps.contains(name)) {
            result.emplace_back(entity);
        }
    }

    return result;
}

bool Scene::onPlayMode(RunPlayModeEvent &e) {
    m_PlayMode = e.getPlaying();
    if (m_PlayMode) {
        Application::get().getEngine()->getSystemManager().loadAll(this);

    }
    else {
        Application::get().getEngine()->getSystemManager().unloadAll();
    }

    return false;
}

Entity Scene::makeEntity(entt::entity entity) {
    return Entity{this, entity};
}

