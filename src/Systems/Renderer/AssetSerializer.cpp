#include "ZeusEngineCore/AssetSerializer.h"
#include <ZeusEngineCore/Application.h>
#include "ZeusEngineCore/SerializerCommon.h"

ZEN::AssetSerializer::AssetSerializer(AssetLibrary *library) : m_AssetLibrary(library){

}

bool ZEN::AssetSerializer::serialize(const std::string &path) {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Assets" << YAML::Value << "Default";

    out << YAML::Key << "Textures" << YAML::BeginSeq;

    for(auto& [name, texture] : m_AssetLibrary->m_Textures) {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << name;

        if (!texture->path.empty()) {
            out << YAML::Key << "Path" << YAML::Value << texture->path;
        }

        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
    out << YAML::Key << "Shaders" << YAML::BeginSeq;
    for(auto& [name, shader] : m_AssetLibrary->m_Shaders) {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << name;

        out << YAML::Key << "VertPath" << YAML::Value << shader->vertPath;
        out << YAML::Key << "FragPath" << YAML::Value << shader->fragPath;
        out << YAML::Key << "GeoPath" << YAML::Value << shader->geoPath;

        out << YAML::EndMap;
    }

    out << YAML::EndSeq;
    out << YAML::Key << "Materials" << YAML::BeginSeq;
    for(auto& [name, mat] : m_AssetLibrary->m_Materials) {
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << name;

        out << YAML::Key << "Shader" << YAML::Value << mat->shader;
        out << YAML::Key << "Texture" << YAML::Value << mat->texture;
        out << YAML::Key << "MetallicTexture" << YAML::Value << mat->metallicTex;
        out << YAML::Key << "RoughnessTexture" << YAML::Value << mat->roughnessTex;
        out << YAML::Key << "NormalTexture" << YAML::Value << mat->normalTex;
        out << YAML::Key << "AoTexture" << YAML::Value << mat->aoTex;
        out << YAML::Key << "Albedo" << YAML::Value << mat->albedo;
        out << YAML::Key << "Metallic" << YAML::Value << mat->metallic;
        out << YAML::Key << "Roughness" << YAML::Value << mat->roughness;
        out << YAML::Key << "AO" << YAML::Value << mat->ao;
        out << YAML::Key << "Metal" << YAML::Value << mat->metal;
        out << YAML::Key << "UseAlbedo" << YAML::Value << mat->useAlbedo;
        out << YAML::Key << "UseMetallic" << YAML::Value << mat->useMetallic;
        out << YAML::Key << "UseRoughness" << YAML::Value << mat->useRoughness;
        out << YAML::Key << "UseNormal" << YAML::Value << mat->useNormal;
        out << YAML::Key << "UseAO" << YAML::Value << mat->useAO;

        out << YAML::EndMap;
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;
    std::ofstream fout(Application::get().getResourceRoot() + path);
    fout << out.c_str();
    return true;
}

bool ZEN::AssetSerializer::deserialize(const std::string &path) {

}
