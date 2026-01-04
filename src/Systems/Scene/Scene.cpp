#include "ZeusEngineCore/Scene.h"

#include <ZeusEngineCore/Application.h>
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/InputEvents.h>
#include <ZeusEngineCore/AssetLibrary.h>
#include <ZeusEngineCore/ZEngine.h>
#include "ZeusEngineCore/SceneSerializer.h"

using namespace ZEN;

Scene::Scene() {
    m_SystemManager.loadAllFromDirectory(Project::getActive()->getActiveProjectRoot() +
        "assets/scripts/bin/", this);
}

void Scene::onUpdate(float dt) {
    if (m_PlayMode) {
        m_SystemManager.updateAll(dt);
    }
}

void Scene::onMeshCompRemove(entt::registry& registry, entt::entity entity) {
    //need to call render systems remove mesh comp somehow
    //Application::get()
    //m_Dispatcher->trigger<RemoveMeshCompEvent>(RemoveMeshCompEvent{Entity(&m_Registry, entity)});
}

void Scene::onMeshDrawableRemove(entt::registry& registry, entt::entity entity) {

    //need to call render systems remove mesh drawable somehow
    //m_Dispatcher->trigger<RemoveMeshDrawableEvent>(RemoveMeshDrawableEvent{Entity(&m_Registry, entity)});
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
}

void* Scene::addRuntimeComponent(entt::entity entity, const ComponentInfo& compInfo) {
    auto& entityMap = m_RuntimeComponents[entity];
    auto& storage = entityMap[compInfo.name];
    storage.buffer.resize(compInfo.size);
    storage.info = &compInfo;
    std::memset(storage.buffer.data(), 0, compInfo.size);
    return storage.buffer.data();
}

RuntimeComponent* Scene::getRuntimeComponent(entt::entity entity, const std::string& compName) {
    auto entityIt = m_RuntimeComponents.find(entity);
    if (entityIt == m_RuntimeComponents.end()) return nullptr;
    auto compIt = entityIt->second.find(compName);
    if (compIt == entityIt->second.end()) return nullptr;
    return &compIt->second;
}

void Scene::removeEntity(Entity entity) {
    m_Registry.destroy((entt::entity)entity);
}



void Scene::onEvent(Event &event) {
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<RemoveResourceEvent>([this](RemoveResourceEvent& e) {return onRemoveResource(e); });
    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayMode(e); });
}

std::vector<Entity> Scene::getEntities(const std::string &name) {
    std::vector<Entity> result;

    for (auto& [entity, comps] : m_RuntimeComponents) {
        if (comps.contains(name)) {
            result.emplace_back(this, entity);
        }
    }

    return result;
}

bool Scene::onPlayMode(RunPlayModeEvent &e) {
    m_PlayMode = e.getPlaying();
    if (m_PlayMode) {
        m_SystemManager.loadAll(this);

    }
    else {
        m_SystemManager.unloadAll();
    }

    return false;
}

Entity Scene::makeEntity(entt::entity entity) {
    return Entity{this, entity};
}

bool Scene::onRemoveResource(RemoveResourceEvent &e) {
    switch(e.getResourceType()) {
        case Resources::MeshDrawable: {
            //todo cleanup gpu resource
            //removeResource<MeshDrawableComp>(e.getResourceName());
            return true;
        }

        case Resources::MeshData: {
            //removeResource<MeshComp>(e.getResourceName());
            return true;
        }

        case Resources::Material: {
            //todo cleanup shader resources
            //removeResource<MaterialComp>(e.getResourceName());
            return true;
        }

        case Resources::Texture: {
            //todo cleanup textures
            break;
        }
        default: {
            std::cout<<"Resource type undefined!"<<"\n";
            return false;
        }
    }
    return false;
}
