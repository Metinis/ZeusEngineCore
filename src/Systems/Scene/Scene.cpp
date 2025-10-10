#include "ZeusEngineCore/Scene.h"
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/InputEvents.h>
#include <ZeusEngineCore/ModelLibrary.h>
#include <ZeusEngineCore/ZEngine.h>

#include "ZeusEngineCore/EventDispatcher.h"

using namespace ZEN;

Scene::Scene(EventDispatcher& dispatcher){
    dispatcher.attach<RemoveMeshEvent, Scene, &Scene::onRemoveMesh>(this);
    dispatcher.attach<RemoveMaterialEvent, Scene, &Scene::onRemoveMaterial>(this);
    dispatcher.attach<RemoveTextureEvent, Scene, &Scene::onRemoveTexture>(this);
}

void Scene::createDefaultScene(const std::string& resourceRoot, ZEngine* engine) {
    auto cameraEntity = createEntity("Scene Camera");
    cameraEntity.addComponent<CameraComp>();

    auto skyboxEntity = createEntity("Skybox");
    SkyboxComp skyboxComp{
        .shaderID = engine->getRenderer().getResourceManager()->createShader(
                resourceRoot + "/shaders/glskybox.vert", resourceRoot + "/shaders/glskybox.frag"),
        .textureID = engine->getRenderer().getResourceManager()->createCubeMapTexture(resourceRoot + "/textures/skybox/"),

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


Entity Scene::makeEntity(entt::entity entity) {
    return Entity{this, entity};
}

void Scene::onRemoveMesh(RemoveMeshEvent& e) {
    auto view = getEntities<MeshComp>();
    for (auto entity : view) {
        if(entity.getComponent<MeshComp>().name != e.meshName) {
            continue;
        }
        entity.removeComponent<MeshComp>();
        if(entity.hasComponent<MeshDrawableComp>()) {
            entity.removeComponent<MeshDrawableComp>();
        }
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
