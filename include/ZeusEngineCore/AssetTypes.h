#pragma once
#include "Vertex.h"
namespace ZEN {
    using AssetID = UUID;
    struct Material {
        AssetID shader{0};
        AssetID texture{0};
        AssetID metallicTex{0};
        AssetID roughnessTex{0};
        AssetID normalTex{0};
        AssetID aoTex{0};
        glm::vec3 albedo{1.0f, 1.0f, 1.0f};
        float metallic{1.0f};
        float roughness{1.0f};
        float ao{1.0f};
        bool metal{false};
        bool useAlbedo{false};
        bool useMetallic{false};
        bool useRoughness{false};
        bool useNormal{false};
        bool useAO{false};
    };
    struct MaterialRaw {
        uint32_t shaderID{0};
        uint32_t textureID{0};
        uint32_t metallicTexID{0};
        uint32_t roughnessTexID{0};
        uint32_t normalTexID{0};
        uint32_t aoTexID{0};
        glm::vec3 albedo{1.0f, 1.0f, 1.0f};
        float metallic{1.0f};
        float roughness{1.0f};
        float ao{1.0f};
        bool metal{false};
        bool useAlbedo{false};
        bool useMetallic{false};
        bool useRoughness{false};
        bool useNormal{false};
        bool useAO{false};
    };

    struct MeshData {
        std::vector<uint32_t> indices{};
        std::vector<Vertex> vertices{};
    };

    enum TextureType {
        Texture2D,
        Texture2DRaw,
        Cubemap,
        CubemapHDR,
        HDR,
        Prefilter,
        BRDF
    };
    struct TextureData {
        std::string path{};
        TextureType type{Texture2D};
        glm::vec2 dimensions{};
        bool mip{false};
        bool absPath{false};
    };
    struct ShaderData {
        std::string vertPath;
        std::string fragPath;
        std::string geoPath;
    };
using AssetVariant = std::variant<
        MeshData,
        Material,
        TextureData,
        ShaderData
    //Add to this for more asset types
    >;
};
