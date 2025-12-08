#include "SceneSerializer.h"
#include <yaml-cpp/yaml.h>
#include <entt/entt.hpp>
#include <ZeusEngineCore/Application.h>

using namespace ZEN;
SceneSerializer::SceneSerializer(Scene *scene) : m_Scene(scene) {

}

static void serializeEntity(YAML::Emitter& out, Entity entity) {
    out << YAML::BeginMap;
    out << YAML::Key << "Entity" << YAML::Value << "671526351623";

    if(entity.hasComponent<TagComp>()) {
        out << YAML::Key << "TagComponent";
        out << YAML::BeginMap;
        out << YAML::Key << "Tag" << YAML::Value << entity.getComponent<TagComp>().tag;
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
    return false;
}
