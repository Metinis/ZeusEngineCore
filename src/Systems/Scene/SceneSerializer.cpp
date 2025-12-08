#include "SceneSerializer.h"
#include <yaml-cpp/yaml.h>
#include <ZeusEngineCore/Application.h>

ZEN::SceneSerializer::SceneSerializer(Scene *scene) : m_Scene(scene) {

}

bool ZEN::SceneSerializer::serialize(const std::string &path) {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << "Default";
    out << YAML::Key << "Entities";

    out << YAML::EndMap;
    std::ofstream fout(Application::get().getResourceRoot() + path);
    fout << out.c_str();
    return true;
}

bool ZEN::SceneSerializer::deserialize(const std::string &path) {
    return false;
}
