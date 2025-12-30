#include "ZeusEngineCore/Scene.h"

#include <ZeusEngineCore/Application.h>
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/InputEvents.h>
#include <ZeusEngineCore/AssetLibrary.h>
#include <ZeusEngineCore/ZEngine.h>
#include "ZeusEngineCore/SceneSerializer.h"

using namespace ZEN;

Scene::Scene() {
}

void Scene::onUpdate(float dt) {
    m_SystemManager.updateAll(dt);
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
        .ambient = m_AmbientColor,
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

    /*
    auto skyboxEntity = createEntity("Skybox");
    SkyboxComp skyboxComp {
        .skyboxMat.name = "Skybox",
        .eqMat.name = "EqMap",
        .conMat.name = "ConMap",
        .prefilterMat.name = "PrefilterMap",
        .brdfLUTMat.name = "brdfLUT"
    };

    skyboxEntity.addComponent<SkyboxComp>(skyboxComp);
    skyboxEntity.addComponent<MeshComp>(MeshComp{.name = "Skybox"});*/

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

void Scene::removeEntity(Entity entity) {
    m_Registry.destroy((entt::entity)entity);
}

void Scene::onEvent(Event &event) {
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<RemoveResourceEvent>([this](RemoveResourceEvent& e) {return onRemoveResource(e); });
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
