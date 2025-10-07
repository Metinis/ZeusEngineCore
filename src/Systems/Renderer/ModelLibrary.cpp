#include "ZeusEngineCore/ModelLibrary.h"

#include <ZeusEngineCore/Renderer.h>

#include "IResourceManager.h"
#include "ZeusEngineCore/Components.h"

using namespace ZEN;

ModelLibrary::ModelLibrary(Renderer* renderer, const std::string& resourceRoot)
: m_Renderer(renderer) {
    m_Materials["Default"] = createDefaultMaterial(resourceRoot, "/shaders/glbasic4.1.vert", "/shaders/glbasic4.1.frag");
    m_Meshes["Cube"]  = createCube();
    m_Meshes["Skybox"] = createSkybox();
    m_Meshes["Sphere"] = createSphere(1.0f, 32, 16);

    m_Textures["Wall"] = m_Renderer->getResourceManager()->createTexture(resourceRoot + "/textures/wall.jpg");
    m_Textures["Container"] = m_Renderer->getResourceManager()->createTexture(resourceRoot + "/textures/container2.png");
    m_Textures["ContainerSpec"] = m_Renderer->getResourceManager()->createTexture(resourceRoot + "/textures/container2_specular.png");

    uint32_t defaultShaderID = m_Materials["Default"]->shaderID;
    Material wallMaterial{
        .shaderID = defaultShaderID,
        .textureIDs = {m_Textures["Wall"]},
    };

    m_Materials["Wall"] = std::make_unique<Material>(wallMaterial);

    Material containerMaterial{
        .shaderID = defaultShaderID,
        .textureIDs = {m_Textures["Container"]},
        .specularTexIDs = {m_Textures["ContainerSpec"]},
    };

    m_Materials["Container"] = std::make_unique<Material>(containerMaterial);
}

void ModelLibrary::addMaterial(const std::string &name, std::unique_ptr<Material> material) {
    m_Materials[name] = std::move(material);
}

Material* ModelLibrary::getMaterial(const std::string &name) {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) return it->second.get();
    std::cout<<"Material not found: "<<name<<"\n";
    return nullptr;
}

void ModelLibrary::addTexture(const std::string &name, uint32_t texID) {
    m_Textures[name] = texID; //user is responsible for manually deleting a texture from resource manager
}

uint32_t ModelLibrary::getTexture(const std::string &name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) return it->second;
    std::cout<<"Texture not found: "<<name<<"\n";
    return 0;
}

Mesh* ModelLibrary::getMesh(const std::string &name) {
    auto it = m_Meshes.find(name);
    if (it != m_Meshes.end()) return it->second.get();
    std::cout<<"Mesh not found: "<<name<<"\n";
    return nullptr;
}
void ModelLibrary::addMesh(const std::string& name, std::unique_ptr<Mesh> mesh) {
    m_Meshes[name] = std::move(mesh);
}

void ModelLibrary::addMesh(const std::string &name, const Mesh &mesh) {
    m_Meshes[name] = std::make_unique<Mesh>(mesh);
}

void ModelLibrary::addMaterial(const std::string &name, const Material &material) {
    m_Materials[name] = std::make_unique<Material>(material);
}


std::unique_ptr<Mesh> ModelLibrary::createCube() {
    Mesh mesh;

    mesh.vertices = {
        // Front face (z = +0.5)
        {{-0.5f,  0.5f,  0.5f}, {0,0,1}, {0.0f, 1.0f}}, // Top-left
        {{ 0.5f,  0.5f,  0.5f}, {0,0,1}, {1.0f, 1.0f}}, // Top-right
        {{ 0.5f, -0.5f,  0.5f}, {0,0,1}, {1.0f, 0.0f}}, // Bottom-right
        {{-0.5f, -0.5f,  0.5f}, {0,0,1}, {0.0f, 0.0f}}, // Bottom-left

        // Back face (z = -0.5)
        {{ 0.5f,  0.5f, -0.5f}, {0,0,-1}, {0.0f, 1.0f}}, // Top-left
        {{-0.5f,  0.5f, -0.5f}, {0,0,-1}, {1.0f, 1.0f}}, // Top-right
        {{-0.5f, -0.5f, -0.5f}, {0,0,-1}, {1.0f, 0.0f}}, // Bottom-right
        {{ 0.5f, -0.5f, -0.5f}, {0,0,-1}, {0.0f, 0.0f}}, // Bottom-left

        // Left face (x = -0.5)
        {{-0.5f,  0.5f, -0.5f}, {-1,0,0}, {0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1,0,0}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1,0,0}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1,0,0}, {0.0f, 0.0f}},

        // Right face (x = +0.5)
        {{ 0.5f,  0.5f,  0.5f}, {1,0,0}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1,0,0}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1,0,0}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1,0,0}, {0.0f, 0.0f}},

        // Top face (y = +0.5)
        {{-0.5f,  0.5f, -0.5f}, {0,1,0}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0,1,0}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0,1,0}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0,1,0}, {0.0f, 0.0f}},

        // Bottom face (y = -0.5)
        {{-0.5f, -0.5f,  0.5f}, {0,-1,0}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0,-1,0}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0,-1,0}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0,-1,0}, {0.0f, 0.0f}}
    };

    mesh.indices = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Back
        4, 5, 6, 6, 7, 4,
        // Left
        8, 9,10,10,11, 8,
        // Right
       12,13,14,14,15,12,
        // Top
       16,17,18,18,19,16,
        // Bottom
       20,21,22,22,23,20
    };
    return std::make_unique<Mesh>(mesh);
}

std::unique_ptr<Mesh> ModelLibrary::createSkybox() {
    Mesh skyboxMesh{};
    skyboxMesh.vertices = {
        {{-1.0f,  1.0f, -1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, // Back
        {{-1.0f, -1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, // Front
        {{-1.0f,  1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}}, // Left
        {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}}, // Right
        {{-1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f,  1.0f}}, // Top
        {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f, -1.0f}}  // Bottom
    };

    skyboxMesh.indices = {
        0,2,1,  2,0,3,
        4,6,5,  6,4,7,
        8,10,9, 10,8,11,
        12,14,13, 14,12,15,
        16,18,17, 18,16,19,
        20,22,21, 22,20,23
    };

    return std::make_unique<Mesh>(skyboxMesh);
}

std::unique_ptr<Material> ModelLibrary::createDefaultMaterial(const std::string& resourceRoot, const std::string& vertPath,
            const std::string& fragPath) {
    uint32_t defaultShaderID = m_Renderer->getResourceManager()->createShader(std::string(resourceRoot) + vertPath,
        resourceRoot + fragPath);
    Material defaultMat {.shaderID = defaultShaderID, .textureIDs = {0}};
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
                (float)j / sectorCount,
                (float)i / stackCount
            };
            sphere.vertices.push_back(v);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                sphere.indices.push_back(k1);
                sphere.indices.push_back(k2);
                sphere.indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                sphere.indices.push_back(k1 + 1);
                sphere.indices.push_back(k2);
                sphere.indices.push_back(k2 + 1);
            }
        }
    }
    return std::make_unique<Mesh>(sphere);
}



