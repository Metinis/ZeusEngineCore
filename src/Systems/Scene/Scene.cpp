#include "ZeusEngineCore/engine/Scene.h"

#include <ZeusEngineCore/core/Application.h>
#include <ZeusEngineCore/engine/Entity.h>
#include <ZeusEngineCore/core/InputEvents.h>
#include <ZeusEngineCore/asset/AssetLibrary.h>
#include <ZeusEngineCore/engine/ZEngine.h>
#include "ZeusEngineCore/engine/SceneSerializer.h"

using namespace ZEN;

Scene::Scene()
    : m_PhysicsSystem(&Application::get().getEngine()->getPhysicsSystem())
{
    std::filesystem::path scriptsBinPath =
        std::filesystem::path(Project::getActive()->getActiveProjectRoot())
        / "assets" / "scripts" / "bin";

    Application::get()
        .getEngine()
        ->getSystemManager()
        .loadAllFromDirectory(scriptsBinPath.string(), this);

    createDefaultScene();
}


Scene::~Scene() {

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

    for (auto e : entities) {
        auto& tc = e.getComponent<TransformComp>();
        glm::mat4 local = tc.getLocalMatrix();

        if (auto parent = e.tryGetComponent<ParentComp>()) {
            auto parentEntity = getEntity(parent->parentID);
            tc.worldMatrix = parentEntity.getComponent<TransformComp>().worldMatrix * local;
        } else {
            tc.worldMatrix = local;
        }
    }
}
void Scene::onUpdate(float dt) {
    updateWorldTransforms();
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
    cameraEntity.getComponent<TransformComp>().localPosition = glm::vec3(0.0f, 0.0f, 5.0f);
    cameraEntity.addComponent<CameraComp>();

    auto cubeEntity = createEntity("Cube");
    auto assetLibrary = Project::getActive()->getAssetLibrary();
    cubeEntity.addComponent<MeshComp>(AssetHandle<MeshData>(defaultCubeID));
    cubeEntity.addComponent<BoxColliderComp>();
    cubeEntity.addComponent<RigidBodyComp>();

    auto skyboxEntity = createEntity("Skybox");
    skyboxEntity.addComponent<SkyboxComp>();
    skyboxEntity.addComponent<MeshComp>(AssetHandle<MeshData>(defaultSkyboxID));
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

Entity Scene::getEntityByRegistryID(uint32_t registryID) {

    for (auto entity : m_Registry.storage<entt::entity>()) {
        if ((uint32_t)entity == registryID) {
            return makeEntity(entity);
        }
    }
    return Entity{};
}

Entity Scene::getSceneCamera() {
    auto view = getEntities<SceneCameraComp>();
    for (auto entity : view) {
        return entity;
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

    return false;
}

Entity Scene::makeEntity(entt::entity entity) {
    return Entity{this, entity};
}

