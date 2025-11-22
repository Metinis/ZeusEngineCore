#include "ZeusEngineCore/ModelLibrary.h"
#include <ZeusEngineCore/InputEvents.h>
#include "IResourceManager.h"
#include "ZeusEngineCore/Components.h"
#include "ZeusEngineCore/EventDispatcher.h"

using namespace ZEN;

ModelLibrary::ModelLibrary(EventDispatcher *dispatcher, IResourceManager *resourceManager,
                           const std::string &resourceRoot)
    : m_Dispatcher(dispatcher), m_ResourceManager(resourceManager) {
    m_Materials["Default"] = createDefaultMaterial("/shaders/pbr.vert", "/shaders/pbr.frag", "");
    m_Meshes["Cube"] = createCube();
    m_Meshes["Skybox"] = createSkybox();
    m_Meshes["Quad"] = createQuad();
    m_Meshes["Sphere"] = createSphere(1.0f, 32, 16);

    m_Textures["Wall"] = m_ResourceManager->createTexture("/textures/wall.jpg", false);
    m_Textures["Container"] = m_ResourceManager->createTexture("/textures/container2.png", false);
    m_Textures["ContainerSpec"] = m_ResourceManager->createTexture("/textures/container2_specular.png", false);

    m_Textures["RustedIron_Albedo"] = m_ResourceManager->createTexture(
        "/textures/rusted_iron/rustediron2_basecolor.png", false);
    m_Textures["RustedIron_Metallic"] = m_ResourceManager->createTexture(
        "/textures/rusted_iron/rustediron2_metallic.png", false);
    m_Textures["RustedIron_Normal"] = m_ResourceManager->createTexture("/textures/rusted_iron/rustediron2_normal.png",
                                                                       false);
    m_Textures["RustedIron_Roughness"] = m_ResourceManager->createTexture(
        "/textures/rusted_iron/rustediron2_roughness.png", false);

    m_Textures["DarkWood_Albedo"] = m_ResourceManager->createTexture(
        "/textures/dark-wood-stain-ue/dark-wood-stain_albedo.png", false);
    m_Textures["DarkWood_Metallic"] = m_ResourceManager->createTexture(
        "/textures/dark-wood-stain-ue/dark-wood-stain_metallic.png", false);
    m_Textures["DarkWood_Normal"] = m_ResourceManager->createTexture(
        "/textures/dark-wood-stain-ue/dark-wood-stain_normal-dx.png", false);
    m_Textures["DarkWood_Roughness"] = m_ResourceManager->createTexture(
        "/textures/dark-wood-stain-ue/dark-wood-stain_roughness.png", false);
    m_Textures["DarkWood_AO"] = m_ResourceManager->createTexture(
    "/textures/dark-wood-stain-ue/dark-wood-stain_ao.png", false);

    m_Textures["Brick_Albedo"] = m_ResourceManager->createTexture(
        "/textures/dirty-red-bricks-ue/dirty-red-bricks_albedo.png", false);
    m_Textures["Brick_Metallic"] = m_ResourceManager->createTexture(
        "/textures/dirty-red-bricks-ue/dirty-red-bricks_metallic.png", false);
    m_Textures["Brick_Normal"] = m_ResourceManager->createTexture(
        "/textures/dirty-red-bricks-ue/dirty-red-bricks_normal-dx.png", false);
    m_Textures["Brick_AO"] = m_ResourceManager->createTexture("/textures/dirty-red-bricks-ue/dirty-red-bricks_ao.png",
                                                              false);
    m_Textures["Brick_Roughness"] = m_ResourceManager->createTexture(
        "/textures/dirty-red-bricks-ue/dirty-red-bricks_roughness.png", false);

    //load env map
    m_Textures["Skybox"] = m_ResourceManager->createCubeMapTextureHDRMip(1024, 1024);

    m_Textures["EqMap"] = m_ResourceManager->createHDRTexture("/env-maps/christmas_photo_studio_04_4k.hdr");

    m_Textures["ConMap"] = m_ResourceManager->createCubeMapTextureHDR(32, 32);

    m_Textures["PrefilterMap"] = m_ResourceManager->createPrefilterMap(128, 128);

    m_Textures["brdfLUT"] = m_ResourceManager->createBRDFLUTTexture(1024, 1024);

    uint32_t normalsShaderID = m_ResourceManager->createShader("/shaders/normal-visual.vert",
                                                               "/shaders/normal-visual.frag",
                                                               "/shaders/normal-visual.geom");
    Material normalsMat{
        .shaderID = normalsShaderID,
    };
    m_Materials["NormalsMat"] = std::make_unique<Material>(normalsMat);

    uint32_t defaultShaderID = m_Materials["Default"]->shaderID;
    Material wallMaterial{
        .shaderID = defaultShaderID,
        .textureID = m_Textures["Wall"],
    };

    m_Materials["Wall"] = std::make_unique<Material>(wallMaterial);

    Material containerMaterial{
        .shaderID = defaultShaderID,
        .textureID = m_Textures["Container"],
    };

    m_Materials["Container"] = std::make_unique<Material>(containerMaterial);

    Material rustedMaterial{
        .shaderID = defaultShaderID,
        .textureID = m_Textures["RustedIron_Albedo"],
        .metallicTexID = m_Textures["RustedIron_Metallic"],
        .normalTexID = m_Textures["RustedIron_Normal"],
        .roughnessTexID = m_Textures["RustedIron_Roughness"],

    };

    m_Materials["RustedMaterial"] = std::make_unique<Material>(rustedMaterial);

    Material darkwoodMaterial{
        .shaderID = defaultShaderID,
        .textureID = m_Textures["DarkWood_Albedo"],
        .metallicTexID = m_Textures["DarkWood_Metallic"],
        .normalTexID = m_Textures["DarkWood_Normal"],
        .roughnessTexID = m_Textures["DarkWood_Roughness"],
        .aoTexID = m_Textures["DarkWood_AO"],
    };

    m_Materials["DarkWoodMaterial"] = std::make_unique<Material>(darkwoodMaterial);

    Material bricks{
        .shaderID = defaultShaderID,
        .textureID = m_Textures["Brick_Albedo"],
        .metallicTexID = m_Textures["Brick_Metallic"],
        .normalTexID = m_Textures["Brick_Normal"],
        .roughnessTexID = m_Textures["Brick_Roughness"],

    };

    m_Materials["BricksMaterial"] = std::make_unique<Material>(bricks);

    Material skyboxMat{
        .shaderID = m_ResourceManager->createShader("/shaders/glskyboxHDR.vert", "/shaders/glskyboxHDR.frag", ""),
        .textureID = m_Textures["Skybox"],
    };
    m_Materials["Skybox"] = std::make_unique<Material>(skyboxMat);

    Material eqMap{
        .shaderID = m_ResourceManager->createShader("/shaders/eq-to-cubemap.vert", "/shaders/eq-to-cubemap.frag", ""),
        .textureID = m_Textures["EqMap"],

    };
    m_Materials["EqMap"] = std::make_unique<Material>(eqMap);

    Material conMap{
        .shaderID = m_ResourceManager->createShader("/shaders/irradiance-con.vert", "/shaders/irradiance-con.frag", ""),
        .textureID = m_Textures["ConMap"],
    };
    m_Materials["ConMap"] = std::make_unique<Material>(conMap);

    Material prefilterMap{
        .shaderID = m_ResourceManager->createShader("/shaders/prefilter.vert", "/shaders/prefilter.frag", ""),
        .textureID = m_Textures["PrefilterMap"],

    };
    m_Materials["PrefilterMap"] = std::make_unique<Material>(prefilterMap);

    Material brdfLUT{
        .shaderID = m_ResourceManager->createShader("/shaders/brdf-con.vert", "/shaders/brdf-con.frag", ""),
        .textureID = m_Textures["brdfLUT"],

    };
    m_Materials["brdfLUT"] = std::make_unique<Material>(brdfLUT);
}

void ModelLibrary::removeMesh(const std::string &name) {
    auto it = m_Meshes.find(name);
    if (it != m_Meshes.end()) {
        m_Meshes.erase(name);
        m_Dispatcher->trigger<RemoveMeshEvent>(RemoveMeshEvent{name});
        return;
    }
    std::cout << "Mesh not found: " << name << "\n";
}

void ModelLibrary::removeMaterial(const std::string &name) {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) {
        m_Materials.erase(name);
        m_Dispatcher->trigger<RemoveMaterialEvent>(RemoveMaterialEvent{name});
        return;
    }
    std::cout << "Material not found: " << name << "\n";
}

void ModelLibrary::removeTexture(const std::string &name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        auto textureID = m_Textures[name];
        //find every material with this texture and set the textureID to 0
        for (auto &[matName, material]: m_Materials) {
            if (material->textureID == textureID) {
                material->textureID = 0;
            }
        }
        m_Textures.erase(name);
        m_ResourceManager->deleteTexture(textureID);
        m_Dispatcher->trigger<RemoveTextureEvent>(RemoveTextureEvent{name});
        return;
    }
    std::cout << "Texture not found: " << name << "\n";
}

void ModelLibrary::addMaterial(const std::string &name, std::unique_ptr<Material> material) {
    m_Materials[name] = std::move(material);
}

Material *ModelLibrary::getMaterial(const std::string &name) {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) return it->second.get();
    std::cout << "Material not found: " << name << "\n";
    return nullptr;
}

void ModelLibrary::addTexture(const std::string &name, uint32_t texID) {
    m_Textures[name] = texID; //user is responsible for manually deleting a texture from resource manager
}

uint32_t ModelLibrary::getTexture(const std::string &name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) return it->second;
    std::cout << "Texture not found: " << name << "\n";
    return 0;
}

Mesh *ModelLibrary::getMesh(const std::string &name) {
    auto it = m_Meshes.find(name);
    if (it != m_Meshes.end()) return it->second.get();
    std::cout << "Mesh not found: " << name << "\n";
    return nullptr;
}

//TODO template this
void ModelLibrary::addMesh(const std::string &name, std::unique_ptr<Mesh> mesh) {
    m_Meshes[name] = std::move(mesh);
}

void ModelLibrary::addMesh(const std::string &name, const Mesh &mesh) {
    m_Meshes[name] = std::make_unique<Mesh>(mesh);
}

void ModelLibrary::addMaterial(const std::string &name, const Material &material) {
    m_Materials[name] = std::make_unique<Material>(material);
}

void computeTangents(Mesh &mesh) {
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

std::unique_ptr<Mesh> ModelLibrary::createCube() {
    Mesh mesh;

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
    return std::make_unique<Mesh>(mesh);
}

std::unique_ptr<Mesh> ModelLibrary::createQuad() {
    Mesh mesh;

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
    return std::make_unique<Mesh>(mesh);
}


std::unique_ptr<Mesh> ModelLibrary::createSkybox() {
    Mesh skyboxMesh{};
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
    return std::make_unique<Mesh>(skyboxMesh);
}

std::unique_ptr<Material> ModelLibrary::createDefaultMaterial(const std::string &vertPath,
                                                              const std::string &fragPath, const std::string &geoPath) {
    uint32_t defaultShaderID = m_ResourceManager->createShader(vertPath, fragPath, geoPath);
    Material defaultMat{.shaderID = defaultShaderID, .textureID = 0};
    return std::make_unique<Material>(defaultMat);
}


std::unique_ptr<Mesh> ModelLibrary::createSphere(float radius, unsigned int sectorCount, unsigned int stackCount) {
    Mesh sphere{};

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
    return std::make_unique<Mesh>(sphere);
}
