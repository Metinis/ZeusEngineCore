#include "ZeusEngineCore/ModelLibrary.h"

#include <ZeusEngineCore/Renderer.h>

#include "IResourceManager.h"
#include "ZeusEngineCore/Components.h"

using namespace ZEN;

ModelLibrary::ModelLibrary(Renderer* renderer, const std::string& resourceRoot)
: m_Renderer(renderer) {
    m_Meshes["Cube"]  = createCube();
    m_Meshes["Skybox"] = createSkybox();
    m_Meshes["Sphere"] = createSphere(1.0f, 32, 16);

    MaterialComp wallMaterial{
        .shaderID = m_Renderer->getDefaultShader().shaderID,
        .textureIDs = {m_Renderer->getResourceManager()->createTexture(resourceRoot + "/textures/wall.jpg")},
    };

    m_Materials["Wall"] = std::make_shared<MaterialComp>(wallMaterial);

    MaterialComp containerMaterial{
        .shaderID = m_Renderer->getDefaultShader().shaderID,
        .textureIDs = {m_Renderer->getResourceManager()->createTexture(resourceRoot + "/textures/container2.png")},
        .specularTexIDs = {m_Renderer->getResourceManager()->createTexture(resourceRoot + "/textures/container2_specular.png")},
    };

    m_Materials["Container"] = std::make_shared<MaterialComp>(containerMaterial);
}

void ModelLibrary::addMaterial(const std::string &name, std::shared_ptr<MaterialComp> material) {
    m_Materials[name] = std::move(material);
}

std::shared_ptr<MaterialComp> ModelLibrary::getMaterial(const std::string &name) {
    auto it = m_Materials.find(name);
    if (it != m_Materials.end()) return it->second;
    return nullptr;
}
std::shared_ptr<MeshComp> ModelLibrary::getMesh(const std::string &name) {
    auto it = m_Meshes.find(name);
    if (it != m_Meshes.end()) return it->second;
    return nullptr;
}
void ModelLibrary::addMesh(const std::string& name, std::shared_ptr<MeshComp> mesh) {
    m_Meshes[name] = std::move(mesh);
}

void ModelLibrary::addMesh(const std::string &name, const MeshComp &mesh) {
    m_Meshes[name] = std::make_shared<MeshComp>(mesh);
}

void ModelLibrary::addMaterial(const std::string &name, const MaterialComp &material) {
    m_Materials[name] = std::make_shared<MaterialComp>(material);
}


std::shared_ptr<MeshComp> ModelLibrary::createCube() {
    MeshComp mesh;

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
    mesh.name = "Cube";
    return std::make_shared<MeshComp>(mesh);
}

std::shared_ptr<MeshComp> ModelLibrary::createSkybox() {
    MeshComp skyboxMesh{};
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
    skyboxMesh.name = "Skybox";

    return std::make_shared<MeshComp>(skyboxMesh);
}


std::shared_ptr<MeshComp> ModelLibrary::createSphere(float radius, unsigned int sectorCount, unsigned int stackCount) {
    MeshComp sphere{};

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
    sphere.name = "Sphere";

    return std::make_shared<MeshComp>(sphere);
}



