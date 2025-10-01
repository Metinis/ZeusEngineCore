#include "ZeusEngineCore/Scene.h"
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/ModelLibrary.h>
#include <ZeusEngineCore/ModelImporter.h>
#include <ZeusEngineCore/ZEngine.h>

using namespace ZEN;

Scene::Scene() {

}

void Scene::createDefaultScene(const std::string& resourceRoot, ZEngine* engine) {
    auto cameraEntity = createEntity("Scene Camera");
    cameraEntity.addComponent<CameraComp>();

    auto skyboxEntity = createEntity("Skybox");
    MeshComp skyboxMesh = *engine->getModelLibrary().getMesh("Skybox");
    SkyboxComp skyboxComp{
        .shaderID = engine->getRenderer().getResourceManager()->createShader(
                resourceRoot + "/shaders/glskybox.vert", resourceRoot + "/shaders/glskybox.frag"),
        .textureID = engine->getRenderer().getResourceManager()->createCubeMapTexture(resourceRoot + "/textures/skybox/"),

    };

    skyboxEntity.addComponent<SkyboxComp>(skyboxComp);
    skyboxEntity.addComponent<MeshComp>(skyboxMesh);

    engine->getModelImporter().loadModel("room", resourceRoot + "/models/vr_art_gallery_room.glb");
}

Entity Scene::createEntity(const std::string& name) {
	auto ret = Entity{this, m_Registry.create()};
    ret.addComponent<TransformComp>();
    TagComp tag {.tag = "Unnamed Entity"};
    if(!name.empty()) {
        tag.tag = name;
    }
    ret.addComponent<TagComp>(tag);
    return ret;
}


entt::dispatcher & Scene::getDispatcher() {
	return m_Dispatcher;
}

Entity Scene::makeEntity(entt::entity entity) {
    return Entity{this, entity};
}
