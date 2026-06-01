#include "ZeusEngineCore/asset/AssetSerializer.h"
#include "ZeusEngineCore/core/Application.h"
#include "ZeusEngineCore/core/Project.h"
#include "ZeusEngineCore/stream/FileStreamReader.h"
#include "ZeusEngineCore/stream/FileStreamWriter.h"
#include "ZeusEngineCore/stream/SerializerCommon.h"


ZEN::AssetSerializer::AssetSerializer(AssetLibrary *library) : m_AssetLibrary(library) {
}

bool ZEN::AssetSerializer::serialize(const std::string &path) {
    //todo need to update texture serialization,
    //todo also might just keep serialization/deserialization in their respective structs for the future
    //save all meshes to binary file
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Assets" << YAML::Value << "Default";

    out << YAML::Key << "Meshes" << YAML::BeginSeq;
    for (auto &ID: m_AssetLibrary->getAllIDsOfType<MeshData>()) {
        std::string localPath = std::format("/assets/meshes/{}.bin", std::to_string(ID));
        FileStreamWriter writer(Project::getActive()->getActiveProjectRoot() + localPath);
        writer.writeVector(m_AssetLibrary->get<MeshData>(ID)->indices);
        writer.writeVector(m_AssetLibrary->get<MeshData>(ID)->vertices);

        out << YAML::BeginMap;
        out << YAML::Key << "Mesh" << YAML::Value << ID;
        out << YAML::Key << "Name" << YAML::Value << m_AssetLibrary->getName(ID);
        out << YAML::Key << "Path" << YAML::Value << localPath;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
    out << YAML::Key << "Textures" << YAML::BeginSeq;
    for (auto &ID: m_AssetLibrary->getAllIDsOfType<TextureData>()) {
        auto tex = m_AssetLibrary->get<TextureData>(ID);

        out << YAML::BeginMap;

        out << YAML::Key << "Texture" << YAML::Value << ID;
        out << YAML::Key << "Name" << YAML::Value << m_AssetLibrary->getName(ID);

        out << YAML::Key << "Path" << YAML::Value << tex->path;
        out << YAML::Key << "Type" << YAML::Value << static_cast<uint32_t>(tex->type);
        out << YAML::Key << "Format" << YAML::Value << static_cast<uint32_t>(tex->format);

        out << YAML::Key << "Dimensions" << YAML::Value << tex->dimensions;
        out << YAML::Key << "Size" << YAML::Value << tex->size;

        out << YAML::Key << "Mip" << YAML::Value << tex->mip;
        out << YAML::Key << "AbsPath" << YAML::Value << tex->absPath;

        out << YAML::Key << "SamplerInfo";
        out << YAML::BeginMap;

        out << YAML::Key << "MagFilter"
                << YAML::Value << static_cast<uint32_t>(tex->samplerInfo.magFilter);

        out << YAML::Key << "MinFilter"
                << YAML::Value << static_cast<uint32_t>(tex->samplerInfo.minFilter);

        out << YAML::Key << "MipmapMode"
                << YAML::Value << static_cast<uint32_t>(tex->samplerInfo.mipmapMode);

        out << YAML::Key << "AddressModeU"
                << YAML::Value << static_cast<uint32_t>(tex->samplerInfo.addressModeU);

        out << YAML::Key << "AddressModeV"
                << YAML::Value << static_cast<uint32_t>(tex->samplerInfo.addressModeV);

        out << YAML::Key << "AddressModeW"
                << YAML::Value << static_cast<uint32_t>(tex->samplerInfo.addressModeW);

        out << YAML::Key << "MipLodBias"
                << YAML::Value << tex->samplerInfo.mipLodBias;

        out << YAML::Key << "MinLod"
                << YAML::Value << tex->samplerInfo.minLod;

        out << YAML::Key << "MaxLod"
                << YAML::Value << tex->samplerInfo.maxLod;

        out << YAML::Key << "AnisotropyEnable"
                << YAML::Value << tex->samplerInfo.anisotropyEnable;

        out << YAML::Key << "MaxAnisotropy"
                << YAML::Value << tex->samplerInfo.maxAnisotropy;

        out << YAML::Key << "CompareEnable"
                << YAML::Value << tex->samplerInfo.compareEnable;

        out << YAML::Key << "CompareOp"
                << YAML::Value << static_cast<uint32_t>(tex->samplerInfo.compareOp);

        out << YAML::Key << "BorderColor"
                << YAML::Value << static_cast<uint32_t>(tex->samplerInfo.borderColor);

        out << YAML::Key << "UnnormalizedCoordinates"
                << YAML::Value << tex->samplerInfo.unnormalizedCoordinates;

        out << YAML::EndMap; // SamplerInfo

        out << YAML::EndMap; // Texture
    }
    out << YAML::EndSeq;
    out << YAML::Key << "Materials" << YAML::BeginSeq;
    for (auto &ID: m_AssetLibrary->getAllIDsOfType<Material>()) {
        out << YAML::BeginMap;

        out << YAML::Key << "Material" << YAML::Value << ID;
        out << YAML::Key << "Name" << YAML::Value << m_AssetLibrary->getName(ID);

        auto mat = m_AssetLibrary->get<Material>(ID);

        out << YAML::Key << "Texture" << YAML::Value << mat->texture;
        out << YAML::Key << "MetallicTexture" << YAML::Value << mat->metallicTex;
        out << YAML::Key << "RoughnessTexture" << YAML::Value << mat->roughnessTex;
        out << YAML::Key << "NormalTexture" << YAML::Value << mat->normalTex;
        out << YAML::Key << "AoTexture" << YAML::Value << mat->aoTex;

        out << YAML::Key << "Albedo" << YAML::Value << mat->albedo;
        out << YAML::Key << "Metallic" << YAML::Value << mat->metallic;
        out << YAML::Key << "Roughness" << YAML::Value << mat->roughness;
        out << YAML::Key << "AO" << YAML::Value << mat->ao;

        out << YAML::Key << "UseAlbedo" << YAML::Value << mat->useAlbedo;
        out << YAML::Key << "UseMetallic" << YAML::Value << mat->useMetallic;
        out << YAML::Key << "UseRoughness" << YAML::Value << mat->useRoughness;
        out << YAML::Key << "UseNormal" << YAML::Value << mat->useNormal;
        out << YAML::Key << "UseAO" << YAML::Value << mat->useAO;

        // PipelineInfo
        out << YAML::Key << "PipelineInfo" << YAML::Value;
        out << YAML::BeginMap;

        out << YAML::Key << "VertexShader"
                << YAML::Value << mat->pipelineInfo.vertexShader;

        out << YAML::Key << "FragmentShader"
                << YAML::Value << mat->pipelineInfo.fragmentShader;

        out << YAML::Key << "Topology"
                << YAML::Value << static_cast<uint32_t>(mat->pipelineInfo.topology);

        out << YAML::Key << "PolygonMode"
                << YAML::Value << static_cast<uint32_t>(mat->pipelineInfo.polygonMode);

        out << YAML::Key << "CullMode"
                << YAML::Value << static_cast<uint32_t>(mat->pipelineInfo.cullMode);

        out << YAML::Key << "FrontFace"
                << YAML::Value << static_cast<uint32_t>(mat->pipelineInfo.frontFace);

        out << YAML::Key << "Samples"
                << YAML::Value << static_cast<uint32_t>(mat->pipelineInfo.samples);

        out << YAML::Key << "MultisamplingEnabled"
                << YAML::Value << mat->pipelineInfo.multisamplingEnabled;

        out << YAML::Key << "BlendingEnabled"
                << YAML::Value << mat->pipelineInfo.blendingEnabled;

        out << YAML::Key << "BlendingAdditive"
                << YAML::Value << mat->pipelineInfo.blendingAdditive;

        out << YAML::Key << "BlendingAlpha"
                << YAML::Value << mat->pipelineInfo.blendingAlpha;

        out << YAML::Key << "DepthTestEnabled"
                << YAML::Value << mat->pipelineInfo.depthTestEnabled;

        out << YAML::Key << "DepthWriteEnabled"
                << YAML::Value << mat->pipelineInfo.depthWriteEnabled;

        out << YAML::Key << "DepthCompareOp"
                << YAML::Value << static_cast<uint32_t>(mat->pipelineInfo.depthCompareOp);

        out << YAML::EndMap; // PipelineInfo

        out << YAML::EndMap; // Material
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;
    std::ofstream fout(Project::getActive()->getActiveProjectRoot() + path);
    fout << out.c_str();
    return true;
}

bool ZEN::AssetSerializer::deserialize(const std::string &path) {
    //need to clear data before
    YAML::Node data;
    try {
        data = YAML::LoadFile(Project::getActive()->getActiveProjectRoot() + path);
    } catch (YAML::Exception &e) {
        return false;
    }

    if (!data["Assets"])
        return false;

    m_AssetLibrary->clearNonDefaults();

    auto meshes = data["Meshes"];
    if (meshes) {
        for (auto mesh: meshes) {
            auto id = mesh["Mesh"];
            auto name = mesh["Name"];
            auto meshpath = mesh["Path"];
            if (id && meshpath) {
                MeshData meshdata{};
                FileStreamReader reader(Project::getActive()->getActiveProjectRoot() + meshpath.as<std::string>());
                if (reader.isStreamGood()) {
                    reader.readVector(meshdata.indices);
                    reader.readVector(meshdata.vertices);
                    m_AssetLibrary->addAsset<MeshData>(id.as<uint64_t>(), std::move(meshdata), name.as<std::string>());
                }
            }
        }
    }
    auto textures = data["Textures"];
    if (textures) {
        for (auto texture: textures) {
            auto id = texture["Texture"];
            auto name = texture["Name"];

            if (id) {
                SamplerInfo samplerInfo{};

                if (auto sampler = texture["SamplerInfo"]) {
                    samplerInfo.magFilter =
                            static_cast<VkFilter>(
                                sampler["MagFilter"].as<uint32_t>());

                    samplerInfo.minFilter =
                            static_cast<VkFilter>(
                                sampler["MinFilter"].as<uint32_t>());

                    samplerInfo.mipmapMode =
                            static_cast<VkSamplerMipmapMode>(
                                sampler["MipmapMode"].as<uint32_t>());

                    samplerInfo.addressModeU =
                            static_cast<VkSamplerAddressMode>(
                                sampler["AddressModeU"].as<uint32_t>());

                    samplerInfo.addressModeV =
                            static_cast<VkSamplerAddressMode>(
                                sampler["AddressModeV"].as<uint32_t>());

                    samplerInfo.addressModeW =
                            static_cast<VkSamplerAddressMode>(
                                sampler["AddressModeW"].as<uint32_t>());

                    samplerInfo.mipLodBias =
                            sampler["MipLodBias"].as<float>();

                    samplerInfo.minLod =
                            sampler["MinLod"].as<float>();

                    samplerInfo.maxLod =
                            sampler["MaxLod"].as<float>();

                    samplerInfo.anisotropyEnable =
                            sampler["AnisotropyEnable"].as<bool>();

                    samplerInfo.maxAnisotropy =
                            sampler["MaxAnisotropy"].as<float>();

                    samplerInfo.compareEnable =
                            sampler["CompareEnable"].as<bool>();

                    samplerInfo.compareOp =
                            static_cast<VkCompareOp>(
                                sampler["CompareOp"].as<uint32_t>());

                    samplerInfo.borderColor =
                            static_cast<VkBorderColor>(
                                sampler["BorderColor"].as<uint32_t>());

                    samplerInfo.unnormalizedCoordinates =
                            sampler["UnnormalizedCoordinates"].as<bool>();
                }

                m_AssetLibrary->addAsset<TextureData>(
                    id.as<uint64_t>(),
                    TextureData{
                        .path = texture["Path"].as<std::string>(),
                        .type = static_cast<TextureType>(
                            texture["Type"].as<uint32_t>()),
                        .format = static_cast<VkFormat>(
                            texture["Format"].as<uint32_t>()),
                        .dimensions = texture["Dimensions"].as<glm::vec2>(),
                        .size = texture["Size"].as<uint32_t>(),
                        .mip = texture["Mip"].as<bool>(),
                        .absPath = texture["AbsPath"].as<bool>(),
                        .samplerInfo = samplerInfo,
                    },
                    name.as<std::string>()
                );
            }
        }
    }
    auto shaders = data["Shaders"];
    if (shaders) {
        for (auto shader: shaders) {
            auto id = shader["Shader"];
            auto name = shader["Name"];
            if (id) {
                m_AssetLibrary->addAsset<ShaderData>(id.as<uint64_t>(),
                                                     ShaderData{
                                                         .vertPath = shader["VertPath"].as<std::string>(),
                                                         .fragPath = shader["FragPath"].as<std::string>(),
                                                         .geoPath = shader["GeoPath"].as<std::string>(),
                                                     },
                                                     name.as<std::string>()
                );
            }
        }
    }
    auto materials = data["Materials"];
    if (materials) {
        for (auto material: materials) {
            auto id = material["Material"];
            auto name = material["Name"];

            if (id) {
                PipelineInfo pipelineInfo{};

                if (auto pipeline = material["PipelineInfo"]) {
                    pipelineInfo.vertexShader = pipeline["VertexShader"].as<std::string>();
                    pipelineInfo.fragmentShader = pipeline["FragmentShader"].as<std::string>();
                    pipelineInfo.topology = static_cast<VkPrimitiveTopology>(
                        pipeline["Topology"].as<uint32_t>());
                    pipelineInfo.polygonMode = static_cast<VkPolygonMode>(
                        pipeline["PolygonMode"].as<uint32_t>());
                    pipelineInfo.cullMode = static_cast<VkCullModeFlags>(
                        pipeline["CullMode"].as<uint32_t>());
                    pipelineInfo.frontFace = static_cast<VkFrontFace>(
                        pipeline["FrontFace"].as<uint32_t>());
                    pipelineInfo.samples = static_cast<VkSampleCountFlagBits>(
                        pipeline["Samples"].as<uint32_t>());

                    pipelineInfo.multisamplingEnabled =
                            pipeline["MultisamplingEnabled"].as<bool>();
                    pipelineInfo.blendingEnabled =
                            pipeline["BlendingEnabled"].as<bool>();
                    pipelineInfo.blendingAdditive =
                            pipeline["BlendingAdditive"].as<bool>();
                    pipelineInfo.blendingAlpha =
                            pipeline["BlendingAlpha"].as<bool>();
                    pipelineInfo.depthTestEnabled =
                            pipeline["DepthTestEnabled"].as<bool>();
                    pipelineInfo.depthWriteEnabled =
                            pipeline["DepthWriteEnabled"].as<bool>();
                    pipelineInfo.depthCompareOp = static_cast<VkCompareOp>(
                        pipeline["DepthCompareOp"].as<uint32_t>());
                }

                Material mat{
                    .texture = AssetID(material["Texture"].as<uint64_t>()),
                    .metallicTex = AssetID(material["MetallicTexture"].as<uint64_t>()),
                    .roughnessTex = AssetID(material["RoughnessTexture"].as<uint64_t>()),
                    .normalTex = AssetID(material["NormalTexture"].as<uint64_t>()),
                    .aoTex = AssetID(material["AoTexture"].as<uint64_t>()),

                    .albedo = material["Albedo"].as<glm::vec3>(),
                    .metallic = material["Metallic"].as<float>(),
                    .roughness = material["Roughness"].as<float>(),
                    .ao = material["AO"].as<float>(),

                    .useAlbedo = material["UseAlbedo"].as<bool>(),
                    .useMetallic = material["UseMetallic"].as<bool>(),
                    .useRoughness = material["UseRoughness"].as<bool>(),
                    .useNormal = material["UseNormal"].as<bool>(),
                    .useAO = material["UseAO"].as<bool>(),

                    .pipelineInfo = std::move(pipelineInfo)
                };

                m_AssetLibrary->addAsset<Material>(
                    id.as<uint64_t>(),
                    std::move(mat),
                    name.as<std::string>());
            }
        }
    }
    return true;
}
