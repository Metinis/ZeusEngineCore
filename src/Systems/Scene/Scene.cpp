#include "ZeusEngineCore/Scene.h"

#include <ZeusEngineCore/MeshLibrary.h>

using namespace ZEN;

Scene::Scene() {

}

void Scene::createDefaultScene(const std::string& resourceRoot, Renderer* renderer) {
	uint32_t textureID = renderer->getResourceManager()->createTexture(
        resourceRoot + "/textures/container2.png");
    uint32_t specularID = renderer->getResourceManager()->createTexture(
        resourceRoot + "/textures/container2_specular.png");
    uint32_t defaultShaderID = renderer->getDefaultShader().shaderID;
    ZEN::MaterialComp comp {
        .shaderID = defaultShaderID,
        .textureID = textureID,
        .specularTexID = specularID,
        .specular = 0.5f,
        .shininess = 32,
    };

    entt::entity entity = createEntity();
    m_Registry.emplace<ZEN::MeshComp>(entity, *ZEN::MeshLibrary::get("Cube"));
    m_Registry.emplace<ZEN::TransformComp>(entity,
        ZEN::TransformComp{.position = {0.0f, 0.0f, -3.0f}});
    m_Registry.emplace<ZEN::MaterialComp>(entity, comp);
    m_Registry.emplace<ZEN::TagComp>(entity, ZEN::TagComp{.tag = "Cube 1"});

    entt::entity entity2 = createEntity();
    m_Registry.emplace<ZEN::MeshComp>(entity2, *ZEN::MeshLibrary::get("Cube"));
    m_Registry.emplace<ZEN::TransformComp>(entity2,
        ZEN::TransformComp{.position = {2.0f, 0.0f, -3.0f}});
    m_Registry.emplace<ZEN::MaterialComp>(entity2, comp);
    m_Registry.emplace<ZEN::TagComp>(entity2, ZEN::TagComp{.tag = "Cube 2"});

    entt::entity cameraEntity = createEntity();
    m_Registry.emplace<ZEN::CameraComp>(cameraEntity);
    m_Registry.emplace<ZEN::TransformComp>(cameraEntity);
    m_Registry.emplace<ZEN::TagComp>(cameraEntity, ZEN::TagComp{.tag = "Scene Camera"});

    entt::entity skyboxEntity = createEntity();
    ZEN::MeshComp skyboxMesh = *ZEN::MeshLibrary::get("Skybox");

    ZEN::SkyboxComp skyboxComp{};
    skyboxComp.shaderID = renderer->getResourceManager()->createShader(
        resourceRoot + "/shaders/glskybox.vert", resourceRoot + "/shaders/glskybox.frag");
    skyboxComp.textureID = renderer->getResourceManager()->createCubeMapTexture(resourceRoot + "/textures/skybox/");
    m_Registry.emplace<ZEN::SkyboxComp>(skyboxEntity, skyboxComp);
    m_Registry.emplace<ZEN::MeshComp>(skyboxEntity, skyboxMesh);

    MeshLibrary::load("test", resourceRoot + "/models/Survival_BackPack_2/Survival_BackPack_2.fbx");
    entt::entity backpack = createEntity();
    m_Registry.emplace<ZEN::MeshComp>(backpack, *ZEN::MeshLibrary::get("test"));
    m_Registry.emplace<ZEN::TransformComp>(backpack,
        ZEN::TransformComp{.position = {4.0f, 0.0f, -3.0f}});
    m_Registry.emplace<ZEN::MaterialComp>(backpack, comp);
    m_Registry.emplace<ZEN::TagComp>(backpack, ZEN::TagComp{.tag = "Backpack"});

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
