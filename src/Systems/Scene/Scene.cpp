#include "ZeusEngineCore/Scene.h"

#include <ZeusEngineCore/ModelLibrary.h>
#include <ZeusEngineCore/ModelImporter.h>
#include <ZeusEngineCore/ZEngine.h>

using namespace ZEN;

Scene::Scene() {

}

void Scene::createDefaultScene(const std::string& resourceRoot, ZEngine* engine) {
	uint32_t textureID = engine->getRenderer().getResourceManager()->createTexture(
        resourceRoot + "/textures/container2.png");
    uint32_t textureID2 = engine->getRenderer().getResourceManager()->createTexture(
        resourceRoot + "/textures/texture.jpg");
    uint32_t specularID = engine->getRenderer().getResourceManager()->createTexture(
        resourceRoot + "/textures/container2_specular.png");
    uint32_t defaultShaderID = engine->getRenderer().getDefaultShader().shaderID;
    ZEN::MaterialComp comp {
        .shaderID = defaultShaderID,
        .textureIDs = {textureID, textureID2},
        .specularTexIDs = {specularID},
        .specular = 0.5f,
        .shininess = 32,
    };
    entt::entity entity = createEntity();
    m_Registry.emplace<ZEN::MeshComp>(entity, *engine->getModelLibrary().getMesh("Cube"));
    m_Registry.emplace<ZEN::TransformComp>(entity,
        ZEN::TransformComp{.localPosition = {0.0f, 0.0f, -3.0f}});
    m_Registry.emplace<ZEN::MaterialComp>(entity, comp);
    m_Registry.emplace<ZEN::TagComp>(entity, ZEN::TagComp{.tag = "Cube 1"});

    entt::entity entity2 = createEntity();
    m_Registry.emplace<ZEN::MeshComp>(entity2, *engine->getModelLibrary().getMesh("Cube"));
    m_Registry.emplace<ZEN::TransformComp>(entity2,
        ZEN::TransformComp{.localPosition = {2.0f, 0.0f, -3.0f}});
    m_Registry.emplace<ZEN::MaterialComp>(entity2, comp);
    m_Registry.emplace<ZEN::TagComp>(entity2, ZEN::TagComp{.tag = "Cube 2"});
    m_Registry.emplace<ZEN::ParentComp>(entity2, ZEN::ParentComp{.parent = entity});

    entt::entity cameraEntity = createEntity();
    m_Registry.emplace<ZEN::CameraComp>(cameraEntity);
    m_Registry.emplace<ZEN::TransformComp>(cameraEntity);
    m_Registry.emplace<ZEN::TagComp>(cameraEntity, ZEN::TagComp{.tag = "Scene Camera"});

    entt::entity skyboxEntity = createEntity();
    ZEN::MeshComp skyboxMesh = *engine->getModelLibrary().getMesh("Skybox");

    ZEN::SkyboxComp skyboxComp{};
    skyboxComp.shaderID = engine->getRenderer().getResourceManager()->createShader(
        resourceRoot + "/shaders/glskybox.vert", resourceRoot + "/shaders/glskybox.frag");
    skyboxComp.textureID = engine->getRenderer().getResourceManager()->createCubeMapTexture(resourceRoot + "/textures/skybox/");
    m_Registry.emplace<ZEN::SkyboxComp>(skyboxEntity, skyboxComp);
    m_Registry.emplace<ZEN::MeshComp>(skyboxEntity, skyboxMesh);

    engine->getModelImporter().loadModel("room", resourceRoot + "/models/vr_art_gallery_room.glb");
    //engine->getModelImporter().loadModel("backpack", resourceRoot + "/models/survival_guitar_backpack4k.glb");

}

entt::entity Scene::createEntity() {
	return m_Registry.create();
}

entt::registry& Scene::getRegistry() {
	return m_Registry;
}

entt::dispatcher & Scene::getDispatcher() {
	return m_Dispather;
}
