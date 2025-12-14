#include "ZeusEngineCore/AssetLibrary.h"
#include <ZeusEngineCore/Application.h>
#include <ZeusEngineCore/InputEvents.h>
#include "IResourceManager.h"
#include "ZeusEngineCore/Components.h"
#include "ZeusEngineCore/AssetHandle.h"

using namespace ZEN;

AssetLibrary::AssetLibrary(IResourceManager *resourceManager, const std::string &resourceRoot) : m_ResourceManager(resourceManager) {
    m_DefaultMatID = createAsset<Material>(createDefaultMaterial("/shaders/pbr.vert", "/shaders/pbr.frag", ""));
    m_CubeID = createAsset<MeshData>(createCube());
    m_SkyboxID = createAsset<MeshData>(createSkybox());
    m_QuadID = createAsset<MeshData>(createQuad());
    m_SphereID = createAsset<MeshData>(createSphere(1.0f, 32, 16));

    /*
    createAndAddDrawable("Cube", *m_MeshData["Cube"]);
    createAndAddDrawable("Skybox", *m_MeshData["Skybox"]);
    createAndAddDrawable("Quad", *m_MeshData["Quad"]);
    createAndAddDrawable("Sphere", *m_MeshData["Sphere"]);

    //load env map
    TextureData data {
        .id = m_ResourceManager->createCubeMapTextureHDRMip(1024, 1024)
    };
    m_Textures["Skybox"] = std::make_unique<TextureData>(data);

    data = {
        .id = m_ResourceManager->createHDRTexture("/env-maps/christmas_photo_studio_04_4k.hdr")
    };
    m_Textures["EqMap"] = std::make_unique<TextureData>(data);

    data = {
        .id = m_ResourceManager->createCubeMapTextureHDR(32, 32)
    };
    m_Textures["ConMap"] = std::make_unique<TextureData>(data);

    data = {
        .id = m_ResourceManager->createPrefilterMap(128, 128)
    };
    m_Textures["PrefilterMap"] = std::make_unique<TextureData>(data);

    data = {
        .id = m_ResourceManager->createBRDFLUTTexture(1024, 1024)
    };
    m_Textures["brdfLUT"] = std::make_unique<TextureData>(data);

    createShader("ScreenQuad", "/shaders/screenQuad.vert", "/shaders/screenQuad.frag", "");
    Material screenQuadMat{
        .shader = "ScreenQuad",
    };
    m_Materials["ScreenQuad"] = std::make_unique<Material>(screenQuadMat);

    createShader("Normals", "/shaders/normal-visual.vert","/shaders/normal-visual.frag",
        "/shaders/normal-visual.geom");
    Material normalsMat{
        .shader = "Normals"
    };
    m_Materials["NormalsMat"] = std::make_unique<Material>(normalsMat);

    Material darkwoodMaterial{
        .shader = "Default",
        .texture = "DarkWood_Albedo",
        .metallicTex = "DarkWood_Metallic",
        .normalTex = "DarkWood_Normal",
        .roughnessTex = "DarkWood_Roughness",
        .aoTex = "DarkWood_AO",
    };

    m_Materials["DarkWoodMaterial"] = std::make_unique<Material>(darkwoodMaterial);

    createShader("HDRSkybox", "/shaders/glskyboxHDR.vert", "/shaders/glskyboxHDR.frag", "");
    Material skyboxMat{
        .shader = "HDRSkybox",
        .texture = "Skybox",
    };
    m_Materials["Skybox"] = std::make_unique<Material>(skyboxMat);

    createShader("EqMap", "/shaders/eq-to-cubemap.vert", "/shaders/eq-to-cubemap.frag", "");
    Material eqMap{
        .shader = "EqMap",
        .texture = "EqMap",

    };
    m_Materials["EqMap"] = std::make_unique<Material>(eqMap);

    createShader("ConMap", "/shaders/irradiance-con.vert", "/shaders/irradiance-con.frag", "");
    Material conMap{
        .shader = "ConMap",
        .texture = "ConMap",
    };
    m_Materials["ConMap"] = std::make_unique<Material>(conMap);

    createShader("PrefilterMap", "/shaders/prefilter.vert", "/shaders/prefilter.frag", "");
    Material prefilterMap{
        .shader = "PrefilterMap",
        .texture = "PrefilterMap",

    };
    m_Materials["PrefilterMap"] = std::make_unique<Material>(prefilterMap);

    createShader("brdfLUT", "/shaders/brdf-con.vert", "/shaders/brdf-con.frag", "");
    Material brdfLUT{
        .shader = "brdfLUT",
        .texture = "brdfLUT",

    };
    m_Materials["brdfLUT"] = std::make_unique<Material>(brdfLUT);

    //AssetSerializer serializer(this);
    //serializer.serialize("/assets/default.zenpackage");
    //serializer.deserialize("/assets/default.zenpackage");
    */
}

/*
void AssetLibrary::removeMeshData(const std::string &name) {
    auto it = m_MeshData.find(name);
    if (it != m_MeshData.end()) {
        m_MeshData.erase(name);
        RemoveResourceEvent event(name, Resources::MeshData);
        Application::get().callEvent(event);
        return;
    }
    std::cout << "Mesh not found: " << name << "\n";
}
void AssetLibrary::removeMeshDrawable(const std::string &name) {
    auto it = m_MeshDrawables.find(name);
    if (it != m_MeshDrawables.end()) {
        m_MeshDrawables.erase(name);
        RemoveResourceEvent event(name, Resources::MeshDrawable);
        Application::get().callEvent(event);
        return;
    }
    std::cout << "Mesh not found: " << name << "\n";
}

void AssetLibrary::createShader(const std::string &name, const std::string &vertPath, const std::string &fragPath,
    const std::string &geoPath) {

    uint32_t shaderID = m_ResourceManager->createShader(vertPath, fragPath, geoPath);

    ShaderData data {
        .vertPath = vertPath,
        .fragPath = fragPath,
        .geoPath = geoPath,
        .id = shaderID
    };
    m_Shaders[name] = std::make_unique<ShaderData>(data);
}

void AssetLibrary::addShader(const std::string &name, std::unique_ptr<ShaderData> shader) {
    m_Shaders[name] = std::move(shader);
}

void AssetLibrary::addShader(const std::string &name, const ShaderData &shader) {
    m_Shaders[name] = std::make_unique<ShaderData>(shader);
}

ShaderData* AssetLibrary::getShader(const std::string &name) {
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) return it->second.get();
    std::cout << "Shader not found, returning default..: " << name << "\n";
    return nullptr;
}

void AssetLibrary::removeShader(const std::string &name) {
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) {
        m_Shaders.erase(name);
        //TODO
        //RemoveResourceEvent event(name, Resources::MeshData);
        //Application::get().callEvent(event);
        return;
    }
    std::cout << "Mesh not found: " << name << "\n";
}
*/
MaterialRaw AssetLibrary::getMaterialRaw(const Material &material) {
    MaterialRaw ret {
        .shaderID = get<ShaderData>(material.shader)->id,
        .textureID = get<TextureData>(material.texture)->id,
        .metallicTexID = get<TextureData>(material.metallicTex)->id,
        .roughnessTexID = get<TextureData>(material.roughnessTex)->id,
        .normalTexID = get<TextureData>(material.normalTex)->id,
        .aoTexID = get<TextureData>(material.aoTex)->id,
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
/*
MaterialRaw AssetLibrary::getMaterialRaw(const std::string &name) {
    auto mat = *m_Materials[name];
    return getMaterialRaw(mat);
}

void AssetLibrary::removeMaterial(const std::string &name) {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) {
        m_Materials.erase(name);
        RemoveResourceEvent event(name, Resources::Material);
        Application::get().callEvent(event);
        return;
    }
    std::cout << "Material not found: " << name << "\n";
}

void AssetLibrary::createTexture(const std::string &name, const std::string &path) {
    TextureData texture = {
        .path = path,
        .id = m_ResourceManager->createTexture(path, false),
    };
    m_Textures[name] = std::make_unique<TextureData>(texture);
}
*/
std::string removePrefix(const std::string& full, const std::string& prefix) {
    if (full.starts_with(prefix)) {
        return full.substr(prefix.size());
    }
    return full;
}
/*
void AssetLibrary::createTextureAbs(const std::string &name, const std::string &path) {
    TextureData texture = {
        .path = removePrefix(path, Application::get().getResourceRoot()),
        .id = m_ResourceManager->createTexture(path, true),
    };
    m_Textures[name] = std::make_unique<TextureData>(texture);
}

void AssetLibrary::removeTexture(const std::string &name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        auto& texture = m_Textures[name];
        //find every material with this texture and set the textureID to 0
        for (auto &[matName, material]: m_Materials) {
            if (material->texture == name) {
                material->texture = "Default";
            }
        }
        m_Textures.erase(name);
        m_ResourceManager->deleteTexture(texture->id);
        RemoveResourceEvent event(name, Resources::Texture);
        Application::get().callEvent(event);
        return;
    }
    std::cout << "Texture not found: " << name << "\n";
}

void AssetLibrary::addMaterial(const std::string &name, std::unique_ptr<Material> material) {
    m_Materials[name] = std::move(material);
}

Material* AssetLibrary::getMaterial(const std::string &name) {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) return it->second.get();
    std::cout << "Material not found, returning default..: " << name << "\n";
    return nullptr;
}

void AssetLibrary::addTexture(const std::string &name, uint32_t texID) {
    m_Textures[name] = std::make_unique<TextureData>(TextureData{.id = texID});
}
void AssetLibrary::addTexture(const std::string &name, const TextureData& data) {
    m_Textures[name] = std::make_unique<TextureData>(data);
}

TextureData* AssetLibrary::getTexture(const std::string &name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) return it->second.get();
    std::cout << "Texture not found, returning default..: " << name << "\n";
    if(name == "Default") {
        std::cout << "ERROR: Default Texture not found!!: " << name << "\n";
        return nullptr;
    }
    return getTexture("Default");
}

MeshData* AssetLibrary::getMeshData(const std::string &name) {
    auto it = m_MeshData.find(name);
    if (it != m_MeshData.end()) return it->second.get();
    std::cout << "Mesh not found, returning default..: " << name << "\n";
    return nullptr;
}

MeshDrawable* AssetLibrary::getMeshDrawable(const std::string &name) {
    auto it = m_MeshDrawables.find(name);
    if (it != m_MeshDrawables.end()) return it->second.get();
    std::cout << "Drawable not found, returning default..: " << name << "\n";
    return nullptr;
}

//TODO template this
void AssetLibrary::addMeshData(const std::string &name, std::unique_ptr<MeshData> mesh) {
    m_MeshData[name] = std::move(mesh);
}

void AssetLibrary::addMeshData(const std::string &name, const MeshData &mesh) {
    m_MeshData[name] = std::make_unique<MeshData>(mesh);
}
void AssetLibrary::addMeshDrawable(const std::string &name, std::unique_ptr<MeshDrawable> drawable) {
    m_MeshDrawables[name] = std::move(drawable);
}

void AssetLibrary::createAndAddDrawable(const std::string &name, const MeshData &data) {
    MeshDrawable drawable {
        .drawableID = m_ResourceManager->createMeshDrawable(data),
        .indexCount = data.indices.size(),
        .instanceCount = 1,
    };
    m_MeshDrawables[name] = std::make_unique<MeshDrawable>(drawable);
}

void AssetLibrary::addMeshDrawable(const std::string &name, const MeshDrawable &drawable) {
    m_MeshDrawables[name] = std::make_unique<MeshDrawable>(drawable);
}

void AssetLibrary::addMaterial(const std::string &name, const Material &material) {
    m_Materials[name] = std::make_unique<Material>(material);
}
*/

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
        .id = 0,
    };
    AssetID defTexID = createAsset<TextureData>(std::move(defaultTex));
    ShaderData defaultShader {
        .vertPath = vertPath,
        .fragPath = fragPath,
        .geoPath = geoPath
    };
    AssetID defShaderID = createAsset<ShaderData>(std::move(defaultShader));
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
