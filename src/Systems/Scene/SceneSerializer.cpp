#include "ZeusEngineCore/SceneSerializer.h"
#include "ZeusEngineCore/SerializerCommon.h"
#include <ZeusEngineCore/Application.h>


using namespace ZEN;
SceneSerializer::SceneSerializer(Scene *scene) : m_Scene(scene) {

}

static void serializeEntity(YAML::Emitter& out, Entity entity) {
    out << YAML::BeginMap;
    assert(entity.hasComponent<UUIDComp>());
    out << YAML::Key << "Entity" << YAML::Value << entity.getComponent<UUIDComp>().uuid;

    if(entity.hasComponent<TagComp>()) {
        out << YAML::Key << "TagComponent";
        out << YAML::BeginMap;
        out << YAML::Key << "Tag" << YAML::Value << entity.getComponent<TagComp>().tag;
        out << YAML::EndMap;
    }
    if(entity.hasComponent<TransformComp>()) {
        out << YAML::Key << "TransformComponent";
        out << YAML::BeginMap;
        auto transformComp = entity.getComponent<TransformComp>();
        out << YAML::Key << "LocalPosition" << YAML::Value << transformComp.localPosition;
        out << YAML::Key << "LocalRotation" << YAML::Value << transformComp.localRotation;
        out << YAML::Key << "LocalScale" << YAML::Value << transformComp.localScale;
        out << YAML::EndMap;
    }
    if(entity.hasComponent<MeshComp>()) {
        out << YAML::Key << "MeshComponent";
        out << YAML::BeginMap;
        out << YAML::Key << "MeshID" << YAML::Value << entity.getComponent<MeshComp>().handle.id();
        out << YAML::EndMap;
    }
    if(entity.hasComponent<MaterialComp>()) {
        out << YAML::Key << "MaterialComponent";
        out << YAML::BeginMap;
        out << YAML::Key << "MaterialID" << YAML::Value << entity.getComponent<MaterialComp>().handle.id();
        out << YAML::EndMap;
    }
    if(entity.hasComponent<DirectionalLightComp>()) {
        out << YAML::Key << "DirLightComponent";
        out << YAML::BeginMap;
        out << YAML::Key << "Ambient" << YAML::Value << entity.getComponent<DirectionalLightComp>().ambient;
        out << YAML::Key << "isPrimary" << YAML::Value << entity.getComponent<DirectionalLightComp>().isPrimary;
        out << YAML::EndMap;
    }
    if(entity.hasComponent<CameraComp>()) {
        out << YAML::Key << "CameraComponent";
        out << YAML::BeginMap;
        out << YAML::Key << "Aspect" << YAML::Value << entity.getComponent<CameraComp>().aspect;
        out << YAML::Key << "Fov" << YAML::Value << entity.getComponent<CameraComp>().fov;
        out << YAML::Key << "Near" << YAML::Value << entity.getComponent<CameraComp>().near;
        out << YAML::Key << "Far" << YAML::Value << entity.getComponent<CameraComp>().far;
        out << YAML::Key << "isPrimary" << YAML::Value << entity.getComponent<CameraComp>().isPrimary;
        out << YAML::EndMap;
    }
    if(entity.hasComponent<ParentComp>()) {
        out << YAML::Key << "ParentComponent";
        out << YAML::BeginMap;
        out << YAML::Key << "ParentID" << YAML::Value << entity.getComponent<ParentComp>().parentID;
        out << YAML::EndMap;
    }
    if(entity.hasComponent<SkyboxComp>()) {
        out << YAML::Key << "SkyboxComponent";
        out << YAML::BeginMap;
        auto skyboxComp = entity.getComponent<SkyboxComp>();
        out << YAML::Key << "SkyboxMaterialID" << YAML::Value << skyboxComp.skyboxMat.handle.id();
        out << YAML::Key << "EqMaterialID" << YAML::Value << skyboxComp.eqMat.handle.id();
        out << YAML::Key << "ConMaterialID" << YAML::Value << skyboxComp.conMat.handle.id();
        out << YAML::Key << "PrefilterMaterialID" << YAML::Value << skyboxComp.prefilterMat.handle.id();
        out << YAML::Key << "BRDFLUTMaterialID" << YAML::Value << skyboxComp.brdfLUTMat.handle.id();
        out << YAML::Key << "EnvGenerated" << YAML::Value << false; //For now, just regenerate
        out << YAML::EndMap;
    }

    out << YAML::EndMap;
}

bool SceneSerializer::serialize(const std::string &path) {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << "Default";
    out << YAML::Key << "Entities" << YAML::BeginSeq;

    for (auto entityID : m_Scene->m_Registry.storage<entt::entity>()) {
        Entity entity = Entity(m_Scene, entityID);
        serializeEntity(out, entity);
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;
    std::ofstream fout(Application::get().getResourceRoot() + path);
    fout << out.c_str();
    return true;
}

bool SceneSerializer::deserialize(const std::string &path) {
    m_Scene->m_Registry.clear();
    YAML::Node data;
    try
    {
        data = YAML::LoadFile(Application::get().getResourceRoot() + path);
    }
    catch (YAML::ParserException e)
    {
        return false;
    }

    if (!data["Scene"])
        return false;

    auto entities = data["Entities"];
    if (entities) {
        for (auto entity : entities) {
            auto id = entity["Entity"].as<uint64_t>();

            std::string name;
            auto tagComponent = entity["TagComponent"];
            if (tagComponent) {
                name = tagComponent["Tag"].as<std::string>();
            }
            auto entityInst = m_Scene->createEntity(name, id);

            auto transformComp = entity["TransformComponent"];
            if (transformComp) {
                TransformComp comp {
                    .localPosition = transformComp["LocalPosition"].as<glm::vec3>(),
                    .localRotation = transformComp["LocalRotation"].as<glm::vec3>(),
                    .localScale = transformComp["LocalScale"].as<glm::vec3>(),
                };
                entityInst.getComponent<TransformComp>() = comp;
            }

            auto meshComp = entity["MeshComponent"];
            if(meshComp) {
                MeshComp comp = {.handle = AssetID(meshComp["MeshID"].as<uint64_t>())};
                entityInst.addComponent<MeshComp>(comp);
            }

            auto matComp = entity["MaterialComponent"];
            if(matComp) {
                MaterialComp comp = {.handle = AssetID(matComp["MaterialID"].as<uint64_t>())};
                entityInst.addComponent<MaterialComp>(comp);
            }

            auto dirLightComp = entity["DirLightComponent"];
            if(dirLightComp) {
                DirectionalLightComp comp = {
                    .ambient = dirLightComp["Ambient"].as<glm::vec3>(),
                    .isPrimary = dirLightComp["isPrimary"].as<bool>(),
                };
                entityInst.addComponent<DirectionalLightComp>(comp);
            }
            auto camComp = entity["CameraComponent"];
            if(camComp) {
                CameraComp comp = {
                    //.projection = glm::perspective(comp.fov, comp.aspect, comp.near, comp.far),
                    .aspect = camComp["Aspect"].as<float>(),
                    .fov = camComp["Fov"].as<float>(),
                    .near = camComp["Near"].as<float>(),
                    .far = camComp["Far"].as<float>(),
                    .isPrimary = camComp["isPrimary"].as<bool>(),
                };
                entityInst.addComponent<CameraComp>(comp);
            }

            auto parentComp = entity["ParentComponent"];
            if(parentComp) {
                ParentComp comp = { .parentID = parentComp["ParentID"].as<uint64_t>()};
                entityInst.addComponent<ParentComp>(comp);
            }

            auto skyboxComp = entity["SkyboxComponent"];
            if(skyboxComp) {
                SkyboxComp comp {
                    .skyboxMat.handle = AssetID(skyboxComp["SkyboxMaterialID"].as<uint64_t>()),
                    .eqMat.handle = AssetID(skyboxComp["EqMaterialID"].as<uint64_t>()),
                    .conMat.handle = AssetID(skyboxComp["ConMaterialID"].as<uint64_t>()),
                    .prefilterMat.handle = AssetID(skyboxComp["PrefilterMaterialID"].as<uint64_t>()),
                    .brdfLUTMat.handle = AssetID(skyboxComp["BRDFLUTMaterialID"].as<uint64_t>()),
                    .envGenerated = skyboxComp["EnvGenerated"].as<bool>(),
                };
                entityInst.addComponent<SkyboxComp>(comp);
            }
        }
    }
    return true;
}
