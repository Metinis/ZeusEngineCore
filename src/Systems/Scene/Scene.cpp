#include "ZeusEngineCore/Scene.h"
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/InputEvents.h>
#include <ZeusEngineCore/ModelLibrary.h>
#include <ZeusEngineCore/ZEngine.h>
#include "ZeusEngineCore/EventDispatcher.h"

using namespace ZEN;

Scene::Scene(EventDispatcher* dispatcher) : m_Dispatcher(dispatcher){
    m_Dispatcher->attach<RemoveMeshEvent, Scene, &Scene::onRemoveMesh>(this);
    m_Dispatcher->attach<RemoveMaterialEvent, Scene, &Scene::onRemoveMaterial>(this);
    m_Dispatcher->attach<RemoveTextureEvent, Scene, &Scene::onRemoveTexture>(this);

    m_Registry.on_destroy<MeshComp>().connect<&Scene::onMeshCompRemove>(this);
    m_Registry.on_destroy<MeshDrawableComp>().connect<&Scene::onMeshDrawableRemove>(this);
}

void Scene::onMeshCompRemove(entt::registry& registry, entt::entity entity) {
    m_Dispatcher->trigger<RemoveMeshCompEvent>(RemoveMeshCompEvent{Entity(&m_Registry, entity)});
}

void Scene::onMeshDrawableRemove(entt::registry& registry, entt::entity entity) {
    m_Dispatcher->trigger<RemoveMeshDrawableEvent>(RemoveMeshDrawableEvent{Entity(&m_Registry, entity)});
}

void Scene::createDefaultScene(ZEngine* engine) {
    auto dirLightEntity = createEntity("Directional Light");
    DirectionalLightComp comp {
        .ambient = m_AmbientColor,
        .isPrimary = true,
    };
    dirLightEntity.addComponent<DirectionalLightComp>(comp);

    auto cameraEntity = createEntity("Scene Camera");
    cameraEntity.addComponent<CameraComp>();

    auto skyboxEntity = createEntity("Skybox");
    SkyboxComp skyboxComp {
        .skyboxMat.name = "Skybox",
        .eqMat.name = "EqMap",
        .conMat.name = "ConMap",
        .prefilterMat.name = "PrefilterMap",
        .brdfLUTMat.name = "brdfLUT"
    };

    skyboxEntity.addComponent<SkyboxComp>(skyboxComp);
    skyboxEntity.addComponent<MeshComp>(MeshComp{.name = "Skybox"});
}

Entity Scene::createEntity(const std::string& name) {
	auto ret = Entity{this, m_Registry.create()};
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

void Scene::removeEntity(Entity entity) {
    //todo remove unused meshes from resource manager
    m_Registry.destroy((entt::entity)entity);
}


Entity Scene::makeEntity(entt::entity entity) {
    return Entity{this, entity};
}

void Scene::onRemoveMesh(RemoveMeshEvent& e) {
    //todo remove unused meshes from resource manager
    auto view = getEntities<MeshComp>();
    for (auto entity : view) {
        if(entity.getComponent<MeshComp>().name != e.meshName) {
            continue;
        }
        entity.removeComponent<MeshComp>();
    }
}

void Scene::onRemoveMaterial(RemoveMaterialEvent& e) {
    auto view = getEntities<MaterialComp>();
    for (auto entity : view) {
        if(entity.getComponent<MaterialComp>().name != e.materialName) {
            continue;
        }
        entity.removeComponent<MaterialComp>();
    }
}

void Scene::onRemoveTexture(RemoveTextureEvent& e) {
    /*auto view = getEntities<MaterialComp>();
    for (auto entity : view) {
    }*/
}


