#include "ZeusEngineCore/Scene.h"

#include <ZeusEngineCore/ModelLibrary.h>

using namespace ZEN;

Scene::Scene() {

}

void Scene::createDefaultScene(const std::string& resourceRoot, Renderer* renderer) {
	uint32_t textureID = renderer->getResourceManager()->createTexture(
        resourceRoot + "/textures/container2.png");
    uint32_t textureID2 = renderer->getResourceManager()->createTexture(
        resourceRoot + "/textures/texture.jpg");
    uint32_t specularID = renderer->getResourceManager()->createTexture(
        resourceRoot + "/textures/container2_specular.png");
    uint32_t defaultShaderID = renderer->getDefaultShader().shaderID;
    ZEN::MaterialComp comp {
        .shaderID = defaultShaderID,
        .textureIDs = {textureID, textureID2},
        .specularTexIDs = {specularID},
        .specular = 0.5f,
        .shininess = 32,
    };

    entt::entity entity = createEntity();
    m_Registry.emplace<ZEN::MeshComp>(entity, *ZEN::ModelLibrary::get("Cube"));
    m_Registry.emplace<ZEN::TransformComp>(entity,
        ZEN::TransformComp{.localPosition = {0.0f, 0.0f, -3.0f}});
    m_Registry.emplace<ZEN::MaterialComp>(entity, comp);
    m_Registry.emplace<ZEN::TagComp>(entity, ZEN::TagComp{.tag = "Cube 1"});

    entt::entity entity2 = createEntity();
    m_Registry.emplace<ZEN::MeshComp>(entity2, *ZEN::ModelLibrary::get("Cube"));
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
    ZEN::MeshComp skyboxMesh = *ZEN::ModelLibrary::get("Skybox");

    ZEN::SkyboxComp skyboxComp{};
    skyboxComp.shaderID = renderer->getResourceManager()->createShader(
        resourceRoot + "/shaders/glskybox.vert", resourceRoot + "/shaders/glskybox.frag");
    skyboxComp.textureID = renderer->getResourceManager()->createCubeMapTexture(resourceRoot + "/textures/skybox/");
    m_Registry.emplace<ZEN::SkyboxComp>(skyboxEntity, skyboxComp);
    m_Registry.emplace<ZEN::MeshComp>(skyboxEntity, skyboxMesh);

    ModelLibrary::load("room", resourceRoot + "/models/vr_art_gallery_room.glb", m_Registry);
    ModelLibrary::load("backpack", resourceRoot + "/models/survival_guitar_backpack4k.glb", m_Registry);

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
