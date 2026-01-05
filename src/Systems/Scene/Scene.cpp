#include "ZeusEngineCore/engine/Scene.h"

#include <ZeusEngineCore/core/Application.h>
#include <ZeusEngineCore/engine/Entity.h>
#include <ZeusEngineCore/core/InputEvents.h>
#include <ZeusEngineCore/asset/AssetLibrary.h>
#include <ZeusEngineCore/engine/ZEngine.h>
#include "ZeusEngineCore/engine/SceneSerializer.h"

using namespace ZEN;

Scene::Scene() {
    Application::get().getEngine()->getSystemManager().loadAllFromDirectory(Project::getActive()->getActiveProjectRoot() +
        "assets/scripts/bin/", this);
    //SceneSerializer serializer(this);
    //m_LoadedScene = serializer.deserialize("assets/scenes/default.zen");
    //if (!m_LoadedScene) {
        createDefaultScene();
    //}
}

Scene::~Scene() {
    //SceneSerializer serializer(this);
    //serializer.serialize("assets/scenes/default.zen");
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

    auto sceneCameraEntity = createEntity("Scene Camera");
    sceneCameraEntity.addComponent<SceneCameraComp>();

    auto cameraEntity = createEntity("Primary Camera");
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
    auto view = getEntities<SceneCameraComp>();
    for (auto entity : view) {
        auto& camera = entity.getComponent<SceneCameraComp>();
        auto& cameraTransform = entity.getComponent<TransformComp>();
        ret.getComponent<TransformComp>() = TransformComp{.localPosition =
                cameraTransform.localPosition + cameraTransform.getFront() * 5.0f};
        break;
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
    auto view = getEntities<SceneCameraComp>();
    for (auto entity : view) {
        auto& camera = entity.getComponent<SceneCameraComp>();
        auto& cameraTransform = entity.getComponent<TransformComp>();
        ret.getComponent<TransformComp>() = TransformComp{.localPosition =
                cameraTransform.localPosition + cameraTransform.getFront() * 5.0f};
        break;
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

bool Scene::isDescendantOf(Entity parent, Entity possibleChild) {
    if (!possibleChild.hasComponent<ParentComp>())
        return false;

    auto current = possibleChild;
    while (current.hasComponent<ParentComp>()) {
        auto pid = current.getComponent<ParentComp>().parentID;
        if (pid == parent.getComponent<UUIDComp>().uuid)
            return true;

        current = getEntity(pid);
        if (!current.isValid())
            break;
    }
    return false;
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

