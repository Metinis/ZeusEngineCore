#include "ZeusEngineCore/asset/AssetLibrary.h"
#include "IResourceManager.h"
#include "ZeusEngineCore/core/Application.h"
#include "ZeusEngineCore/asset/AssetSerializer.h"
#include "ZeusEngineCore/engine/Components.h"

using namespace ZEN;

AssetLibrary::AssetLibrary() : m_ResourceManager(Application::get().getEngine()->getRenderer().getResourceManager()) {
    m_DefaultMatID = createAsset<Material>(
        createDefaultMaterial("/shaders/pbr.vert", "/shaders/pbr.frag", ""),
        "Default");
    m_CubeID = createAsset<MeshData>(createCube(), "Cube");
    m_SkyboxID = createAsset<MeshData>(createSkybox(), "Skybox");
    m_QuadID = createAsset<MeshData>(createQuad(), "Quad");
    m_SphereID = createAsset<MeshData>(createSphere(1.0f, 32, 16), "Sphere");
    //AssetSerializer serializer(this);
    //serializer.deserialize("assets/default.zenpackage");
}

AssetLibrary::~AssetLibrary() {
    //AssetSerializer serializer(this);
    //serializer.serialize("assets/default.zenpackage");
}

MaterialRaw AssetLibrary::getMaterialRaw(const Material &material) {
    AssetHandle<Material> def = getDefaultMaterialID();
    uint32_t shaderID = m_ResourceManager->get<GPUShader>(def->shader)->drawableID;
    if (m_ResourceManager->get<GPUShader>(material.shader)) {
        auto shader = m_ResourceManager->get<GPUShader>(material.shader);
        shaderID = shader->drawableID;
    }
    MaterialRaw ret {
        .shaderID = shaderID,
        .textureID = m_ResourceManager->get<GPUTexture>(material.texture)->drawableID,
        .metallicTexID = m_ResourceManager->get<GPUTexture>(material.metallicTex)->drawableID,
        .roughnessTexID = m_ResourceManager->get<GPUTexture>(material.roughnessTex)->drawableID,
        .normalTexID = m_ResourceManager->get<GPUTexture>(material.normalTex)->drawableID,
        .aoTexID = m_ResourceManager->get<GPUTexture>(material.aoTex)->drawableID,
        .albedo = material.albedo,
        .metallic = material.metallic,
        .roughness = material.roughness,
        .metal = material.metal,
        .useAlbedo = material.useAlbedo,
        .useMetallic = material.useMetallic,
        .useRoughness = material.useRoughness,
        .useNormal = material.useNormal,
        .useAO = material.useAO,
    };
    return ret;
}

MaterialRaw AssetLibrary::getMaterialRaw(const AssetID &material) {
    auto mat = get<Material>(material);
    return getMaterialRaw(*mat);
}
std::string removePrefix(const std::string& full, const std::string& prefix) {
    if (full.starts_with(prefix)) {
        return full.substr(prefix.size());
    }
    return full;
}
void computeTangents(MeshData &mesh) {
    for (auto &v: mesh.vertices) {
        v.Tangent = glm::vec3(0.0f);
        v.Bitangent = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        Vertex &v0 = mesh.vertices[mesh.indices[i + 0]];
        Vertex &v1 = mesh.vertices[mesh.indices[i + 1]];
        Vertex &v2 = mesh.vertices[mesh.indices[i + 2]];

        glm::vec3 edge1 = v1.Position - v0.Position;
        glm::vec3 edge2 = v2.Position - v0.Position;
        glm::vec2 deltaUV1 = v1.TexCoords - v0.TexCoords;
        glm::vec2 deltaUV2 = v2.TexCoords - v0.TexCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent, bitangent;

        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        v0.Tangent += tangent;
        v1.Tangent += tangent;
        v2.Tangent += tangent;
        v0.Bitangent += bitangent;
        v1.Bitangent += bitangent;
        v2.Bitangent += bitangent;
    }

    for (auto &v: mesh.vertices) {
        v.Tangent = glm::normalize(v.Tangent);
        v.Bitangent = glm::normalize(v.Bitangent);
    }
}

MeshData AssetLibrary::createCube() {
    MeshData mesh;

    mesh.vertices = {
        {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0.0f, 1.0f}}, // Front
        {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0.0f, 0.0f}},

        {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0.0f, 0.0f}},

        {{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0.0f, 0.0f}},

        {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0.0f, 0.0f}},

        {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0.0f, 0.0f}},

        {{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0.0f, 0.0f}}
    };

    mesh.indices = {
        0, 2, 1, 3, 2, 0, // Front
        4, 6, 5, 7, 6, 4, // Back
        8, 10, 9, 11, 10, 8, // Left
        12, 14, 13, 15, 14, 12, // Right
        16, 18, 17, 19, 18, 16, // Top
        20, 22, 21, 23, 22, 20 // Bottom
    };
    computeTangents(mesh);
    return mesh;
}

MeshData AssetLibrary::createQuad() {
    MeshData mesh;

    mesh.vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0, 0, 1}, {0.0f, 0.0f}}, // Bottom-left
        {{1.0f, -1.0f, 0.0f}, {0, 0, 1}, {1.0f, 0.0f}}, // Bottom-right
        {{1.0f, 1.0f, 0.0f}, {0, 0, 1}, {1.0f, 1.0f}}, // Top-right
        {{-1.0f, 1.0f, 0.0f}, {0, 0, 1}, {0.0f, 1.0f}} // Top-left
    };

    mesh.indices = {
        0, 1, 2,
        2, 3, 0
    };
    computeTangents(mesh);
    return mesh;
}


MeshData AssetLibrary::createSkybox() {
    MeshData skyboxMesh{};
    skyboxMesh.vertices = {
        {{-1.0f, 1.0f, -1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{1.0f, -1.0f, -1.0f}}, {{1.0f, 1.0f, -1.0f}}, // Back
        {{-1.0f, -1.0f, 1.0f}}, {{-1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f}}, {{1.0f, -1.0f, 1.0f}}, // Front
        {{-1.0f, 1.0f, 1.0f}}, {{-1.0f, -1.0f, 1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, 1.0f, -1.0f}}, // Left
        {{1.0f, 1.0f, -1.0f}}, {{1.0f, -1.0f, -1.0f}}, {{1.0f, -1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f}}, // Right
        {{-1.0f, 1.0f, 1.0f}}, {{-1.0f, 1.0f, -1.0f}}, {{1.0f, 1.0f, -1.0f}}, {{1.0f, 1.0f, 1.0f}}, // Top
        {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f, 1.0f}}, {{1.0f, -1.0f, 1.0f}}, {{1.0f, -1.0f, -1.0f}} // Bottom
    };

    skyboxMesh.indices = {
        0, 1, 2, 0, 2, 3, // Back (flipped)
        4, 5, 6, 4, 6, 7, // Front
        8, 9, 10, 8, 10, 11, // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Top
        20, 21, 22, 20, 22, 23 // Bottom
    };
    return skyboxMesh;
}

Material AssetLibrary::createDefaultMaterial(const std::string &vertPath,
                                                              const std::string &fragPath, const std::string &geoPath) {
    TextureData defaultTex {
        .type = Texture2DRaw,
    };
    AssetID defTexID = createAsset<TextureData>(std::move(defaultTex), "Default");
    ShaderData defaultShader {
        .vertPath = vertPath,
        .fragPath = fragPath,
        .geoPath = geoPath
    };
    AssetID defShaderID = createAsset<ShaderData>(std::move(defaultShader), "Default");
    Material defaultMat{.shader = defShaderID, .texture = defTexID};
    return defaultMat;
}


MeshData AssetLibrary::createSphere(float radius, unsigned int sectorCount, unsigned int stackCount) {
    MeshData sphere{};

    const float PI = 3.14159265359f;

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * (PI / stackCount);
        float xy = radius * cosf(stackAngle);
        float y = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * (2 * PI / sectorCount);

            float x = xy * cosf(sectorAngle);
            float z = xy * sinf(sectorAngle);

            Vertex v{};
            v.Position = {x, y, z};
            v.Normal = glm::normalize(glm::vec3{x, y, z});
            v.TexCoords = {
                (float) j / sectorCount,
                (float) i / stackCount
            };
            sphere.vertices.push_back(v);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                sphere.indices.push_back(k1 + 1);
                sphere.indices.push_back(k2);
                sphere.indices.push_back(k1);
            }

            if (i != (stackCount - 1)) {
                sphere.indices.push_back(k2 + 1);
                sphere.indices.push_back(k2);
                sphere.indices.push_back(k1 + 1);
            }
        }
    }
    computeTangents(sphere);
    return sphere;
}
