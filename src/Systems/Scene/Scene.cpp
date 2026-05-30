#include "ZeusEngineCore/engine/Scene.h"

#include <ZeusEngineCore/core/Application.h>
#include <ZeusEngineCore/engine/Entity.h>
#include <ZeusEngineCore/core/InputEvents.h>
#include <ZeusEngineCore/asset/AssetLibrary.h>

#include "Systems/Renderer/Vulkan/VkHelpers.h"
#include "ZeusEngineCore/engine/CameraSystem.h"
#include "ZeusEngineCore/engine/SceneSerializer.h"
#include "ZeusEngineCore/engine/rendering/VKRenderer.h"

using namespace ZEN;

Scene::Scene(){
}
Scene::~Scene() {

}

void Scene::init(EngineContext *ctx) {
    m_PhysicsSystem = ctx->physicsSystem;
    m_Renderer = ctx->vkRenderer.get();
    m_SystemManager = ctx->systemManager.get();
    m_ModelLibrary = Project::getActive()->getAssetLibrary().get();
    m_CameraSystem = ctx->cameraSystem;
    std::filesystem::path scriptsBinPath =
        std::filesystem::path(Project::getActive()->getActiveProjectRoot())
        / "assets" / "scripts" / "bin";
    ctx->systemManager->loadAllFromDirectory(scriptsBinPath.string(), this);
}

int Scene::computeDepth(Entity e) {
    int depth = 0;
    while (e.hasComponent<ParentComp>()) {
        e = getEntity(e.getComponent<ParentComp>().parentID);
        if (!e.isValid()) break;
        depth++;
    }
    return depth;
}

void Scene::updateWorldTransforms() {
    std::vector<Entity> entities;
    for (auto e : getEntities<TransformComp>())
        entities.push_back(e);

    std::ranges::sort(entities,
                      [&](Entity a, Entity b) {
                          return computeDepth(a) < computeDepth(b);
                      });

    for (auto e : getEntities<TransformComp>()) {
        auto& tc = e.getComponent<TransformComp>();
        glm::mat4 local = tc.getLocalMatrix();

        if (auto parent = e.tryGetComponent<ParentComp>()) {
            auto parentEntity = getEntity(parent->parentID);
            if (parentEntity.isValid())
                tc.worldMatrix = parentEntity.getComponent<TransformComp>().worldMatrix * local;
        } else {
            tc.worldMatrix = local;
        }
    }
}
void Scene::onUpdate(float dt) {
    updateWorldTransforms();
    if (m_PlayMode) {
        m_SystemManager->updateAll(dt);
        for (auto& e : m_PendingCollisionEvents) {
            m_SystemManager->collisionAll(e);
        }
        m_PendingCollisionEvents.clear();
    }
}

void Scene::onRender() {
    for (auto e : getEntities<TransformComp, MeshComp, MaterialComp>()) {
        const auto& tc = e.getComponent<TransformComp>();
        const auto& mc = e.getComponent<MeshComp>();
        const auto& mat = e.getComponent<MaterialComp>();
        m_Renderer->submitDrawCall({mc.handle.id(), mat.handle.id(), tc.worldMatrix});
    }
}


void Scene::createDefaultScene() {
    auto dirLightEntity = createEntity("Directional Light");
    DirectionalLightComp comp {
        .ambient = {0.5f, 0.5f, 0.5f},
        .isPrimary = true,
    };
    dirLightEntity.addComponent<DirectionalLightComp>(comp);

    auto sceneCameraEntity = createEntity("Scene Camera");
    sceneCameraEntity.addComponent<SceneCameraComp>();

    auto cameraEntity = createEntity("Primary Camera");
    cameraEntity.getComponent<TransformComp>().localPosition = glm::vec3(0.0f, 0.0f, 5.0f);
    cameraEntity.addComponent<CameraComp>();

    auto cubeEntity = createEntity("Cube");
    cubeEntity.addComponent<MeshComp>(AssetHandle<MeshData>(defaultSkyboxID));
    cubeEntity.addComponent<BoxColliderComp>();
    cubeEntity.addComponent<RigidBodyComp>();

    TextureData texData{
        .path = Project::getActive()->getActiveProjectRoot() + "/assets/textures/skybox/",
        .type = Cubemap,
        .samplerInfo = VKHelpers::getCubeMapSamplerInfo(),
    };
    auto texId = m_ModelLibrary->createAsset<TextureData>(std::move(texData));
    Material mat {
        .texture = texId,
        .pipelineInfo = PipelineInfo{
            .vertexShader = "/shaders/vulkan-shaders/skybox.vert.spv",
            .fragmentShader = "/shaders/vulkan-shaders/skybox.frag.spv",
            .depthTestEnabled = false,
            .depthWriteEnabled = false,
        }
    };
    mat.useAlbedo = true;
    auto matID = m_ModelLibrary->createAsset<Material>(std::move(mat));
    cubeEntity.addComponent<MaterialComp>(MaterialComp{matID});

    /*for (int i{}; i < 999; ++i) {
        auto cubeEntity = createEntity("Cube " + std::to_string(i));
        cubeEntity.getComponent<TransformComp>().localPosition = glm::vec3(i, 0.0f, i);
        cubeEntity.addComponent<MeshComp>(AssetHandle<MeshData>(defaultCubeID));
        cubeEntity.addComponent<BoxColliderComp>();
        cubeEntity.addComponent<RigidBodyComp>();
        cubeEntity.addComponent<MaterialComp>(MaterialComp{matID});
    }*/

    //auto skyboxEntity = createEntity("Skybox");
    //skyboxEntity.addComponent<SkyboxComp>();
    //skyboxEntity.addComponent<MeshComp>(AssetHandle<MeshData>(defaultSkyboxID));
}

void Scene::onCollisionEnter(Entity a, Entity b, glm::vec3 contactNormal) {
    m_PendingCollisionEvents.emplace_back(CollisionEvent{a, b, contactNormal, CollisionEvent::Type::Enter});
}

void Scene::onCollisionStay(Entity a, Entity b, glm::vec3 contactNormal) {
    m_PendingCollisionEvents.emplace_back(CollisionEvent{a, b, contactNormal, CollisionEvent::Type::Stay});
}

void Scene::onCollisionExit(Entity a, Entity b, glm::vec3 contactNormal) {
    m_PendingCollisionEvents.emplace_back(CollisionEvent{a, b, contactNormal, CollisionEvent::Type::Exit});
}

glm::vec3 Scene::getLightDir() {
    auto view = getEntities<DirectionalLightComp, TransformComp>();
    for (auto entity : view) {
        if (entity.getComponent<DirectionalLightComp>().isPrimary) {
            return entity.getComponent<TransformComp>().getWorldPosition();
        }
    }
    spdlog::warn("No light entity found!");
    return {};
}

Entity Scene::createEntity(const std::string& name) {
	auto ret = Entity{this, m_Registry.create()};
    auto id = ret.addComponent<UUIDComp>();
    m_UUIDToEntityMap.emplace(id.uuid, ret);
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
    m_UUIDToEntityMap.emplace(id, ret);
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
    if (m_UUIDToEntityMap.contains(id)) {
        return m_UUIDToEntityMap.at(id);
    }
    auto view = getEntities<UUIDComp>();
    for (auto entity : view) {
        if (entity.getComponent<UUIDComp>().uuid == id) {
            m_UUIDToEntityMap.emplace(id, entity);
            return entity;
        }
    }
    spdlog::warn("No entity found for UUID!: {}", (uint64_t)id);
    return Entity{};
}

Entity Scene::getEntityByRegistryID(uint32_t registryID) {

    for (auto entity : m_Registry.storage<entt::entity>()) {
        if ((uint32_t)entity == registryID) {
            return makeEntity(entity);
        }
    }
    return Entity{};
}

Entity Scene::getCamera() {
    if (m_CameraSystem->getUseMainCamera()) {
        auto view = getEntities<CameraComp>();
        for (auto entity : view) {
            return entity;
        }
    }
    return getSceneCamera();
}

Entity Scene::getSceneCamera() {
    auto view = getEntities<SceneCameraComp>();
    for (auto entity : view) {
        return entity;
    }
    return Entity{};
}

void Scene::removeEntity(Entity entity) {
    if (m_UUIDToEntityMap.contains(entity.getComponent<UUIDComp>().uuid)) {
        m_UUIDToEntityMap.erase(entity.getComponent<UUIDComp>().uuid);
    }
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

std::vector<Entity> Scene::getEntities(
    std::string_view componentName) const {
    std::vector<Entity> ret;

    std::string n(componentName);

    for (auto& [entity, comps] : m_RuntimeComponents) {
        if (comps.contains(n)) {
            ret.push_back(entity);
        }
    }

    return ret;
}

bool Scene::onPlayMode(RunPlayModeEvent &e) {
    m_PlayMode = e.getPlaying();
    return false;
}

Entity Scene::makeEntity(entt::entity entity) {
    return Entity{this, entity};
}

