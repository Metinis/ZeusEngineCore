#include "ZeusEngineCore/AssetSerializer.h"
#include "ZeusEngineCore/Application.h"
#include "ZeusEngineCore/FileStreamReader.h"
#include "ZeusEngineCore/FileStreamWriter.h"
#include "ZeusEngineCore/SerializerCommon.h"

ZEN::AssetSerializer::AssetSerializer(AssetLibrary *library) : m_AssetLibrary(library){

}

bool ZEN::AssetSerializer::serialize(const std::string &path) {
    //save all meshes to binary file
    /*
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Assets" << YAML::Value << "Default";

    out << YAML::Key << "Meshes" << YAML::BeginSeq;
    for(auto& [name, mesh] : m_AssetLibrary->m_AssetMap) {
        std::string localPath = std::format("/meshes/{}.bin", name);
        FileStreamWriter writer(Application::get().getResourceRoot() + localPath);
        writer.writeVector(mesh->indices);
        writer.writeVector(mesh->vertices);

        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << name;
        out << YAML::Key << "Path" << YAML::Value << localPath;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
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
    fout << out.c_str();*/
    return true;
}

bool ZEN::AssetSerializer::deserialize(const std::string &path) {
    //need to clear data before
    /*
    YAML::Node data;
    try
    {
        data = YAML::LoadFile(Application::get().getResourceRoot() + path);
    }
    catch (YAML::ParserException e)
    {
        return false;
    }

    if (!data["Assets"])
        return false;

    auto meshes = data["Meshes"];
    if(meshes) {
        for(auto mesh : meshes) {
            auto name = mesh["Name"];
            auto meshpath = mesh["Path"];
            if(name && meshpath) {
                MeshData meshdata{};
                FileStreamReader reader(Application::get().getResourceRoot() + meshpath.as<std::string>());
                reader.readVector(meshdata.indices);
                reader.readVector(meshdata.vertices);
                m_AssetLibrary->m_MeshData[name.as<std::string>()] = std::make_unique<MeshData>(meshdata);
            }
        }
    }
    auto textures = data["Textures"];
    if (textures) {
        for(auto texture : textures) {
            auto name = texture["Name"];
            auto texpath = texture["Path"];
            if(name && texpath) {
                m_AssetLibrary->createTexture(name.as<std::string>(), texpath.as<std::string>());
            }
        }
    }
    auto shaders = data["Shaders"];
    if(shaders) {
        for(auto shader : shaders) {
            auto name = shader["Name"];
            if(name) {
                m_AssetLibrary->createShader(name.as<std::string>(), shader["VertPath"].as<std::string>(),
                    shader["FragPath"].as<std::string>(), shader["GeoPath"].as<std::string>());
            }
        }
    }
    auto materials = data["Materials"];
    if(materials) {
        for(auto material : materials) {
            auto name = material["Name"];
            if(name) {
                Material mat {
                    .shader = material["Shader"].as<std::string>(),
                    .texture = material["Texture"].as<std::string>(),
                    .metallicTex = material["MetallicTexture"].as<std::string>(),
                    .normalTex = material["NormalTexture"].as<std::string>(),
                    .roughnessTex = material["RoughnessTexture"].as<std::string>(),
                    .aoTex = material["AoTexture"].as<std::string>(),
                    .albedo = material["Albedo"].as<glm::vec3>(),
                    .metallic = material["Metallic"].as<float>(),
                    .roughness = material["Roughness"].as<float>(),
                    .ao = material["AO"].as<float>(),
                    .metal = material["Metal"].as<bool>(),
                    .useAlbedo = material["UseAlbedo"].as<bool>(),
                    .useMetallic = material["UseMetallic"].as<bool>(),
                    .useRoughness = material["UseRoughness"].as<bool>(),
                    .useNormal = material["UseNormal"].as<bool>(),
                    .useAO = material["UseAO"].as<bool>(),

                };
                m_AssetLibrary->m_Materials[name.as<std::string>()] = std::make_unique<Material>(mat);
            }
        }
    }
    //TODO meshes
    */
    return true;
}
