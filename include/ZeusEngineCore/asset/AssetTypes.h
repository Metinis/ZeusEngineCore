#pragma once
#include "ZeusEngineCore/engine/UUID.h"
#include "ZeusEngineCore/core/Vertex.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#ifdef GENERATE_ENUM_STRINGS
#include <string.h>

#define DECL_ENUM_STRING(e) #e,

#define BEGIN_ENUM_STRINGS(ENUM_NAME) \
static const char* gs_##ENUM_NAME[] = {

#define END_ENUM_STRINGS(ENUM_NAME) \
}; \
inline const char* getString##ENUM_NAME(ENUM_NAME value) { \
return gs_##ENUM_NAME[(int)value]; \
} \
inline bool getEnum##ENUM_NAME##FromString(const char* str, ENUM_NAME* out) { \
for (size_t i = 0; i < sizeof(gs_##ENUM_NAME)/sizeof(gs_##ENUM_NAME[0]); ++i) { \
if (strcmp(gs_##ENUM_NAME[i], str) == 0) { \
*out = (ENUM_NAME)i; \
return true; \
} \
} \
return false; \
}
#endif

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

        glm::vec3 getHalfExtents(glm::vec3& outCenter) const {
            if (vertices.empty()) {
                outCenter = glm::vec3(0.0f);
                return glm::vec3(0.5f);
            }
            glm::vec3 min = vertices[0].position;
            glm::vec3 max = vertices[0].position;

            for (auto vert : vertices) {
                min = glm::min(min, vert.position);
                max = glm::max(max, vert.position);
            }
            outCenter = (min + max) * 0.5f;
            return (max - min) * 0.5f;
        }
        float getRadius(glm::vec3& outCenter) const {
            if (vertices.empty()) {
                outCenter = glm::vec3(0.0f);
                return 0.5f;
            }
            glm::vec3 min = vertices[0].position;
            glm::vec3 max = vertices[0].position;

            for (auto vert : vertices) {
                min = glm::min(min, vert.position);
                max = glm::max(max, vert.position);
            }
            outCenter = (min + max) * 0.5f;

            float radius = 0.0f;
            for (const auto& vert : vertices) {
                radius = glm::max(radius, glm::distance(outCenter, vert.position));
            }

            return radius;
        }
    };


    #define TEXTURE_TYPE_LIST(X) \
        X(Texture2D)             \
        X(Texture2DRaw)          \
        X(Texture2DAssimp)       \
        X(Cubemap)               \
        X(CubemapHDR)            \
        X(HDR)                   \
        X(Prefilter)             \
        X(BRDF)

    #define DECL_ENUM(e) e,

    enum TextureType {
        TEXTURE_TYPE_LIST(DECL_ENUM)
        TextureType_Count
    };

    #undef DECL_ENUM
    const char* getStringTextureType(TextureType type);
    bool getEnumTextureTypeFromString(const char* str, TextureType* out);

    struct TextureData {
        std::string path{};
        TextureType type{Texture2D};
        glm::vec2 dimensions{};
        aiTexture* aiTex{nullptr};
        uint32_t size{};
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
