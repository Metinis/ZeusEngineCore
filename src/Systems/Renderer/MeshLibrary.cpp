#include <utility>

#include "ZeusEngineCore/MeshLibrary.h"

#include <iostream>

#include "ZeusEngineCore/Components.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


using namespace ZEN;

namespace ZEN {
    std::unordered_map<std::string, std::shared_ptr<MeshComp>> MeshLibrary::s_Meshes;
}


void MeshLibrary::init() {
    s_Meshes["Cube"]  = createCube();
    s_Meshes["Skybox"] = createSkybox();
    s_Meshes["Sphere"] = createSphere(1.0f, 32, 16);
}
std::shared_ptr<MeshComp> MeshLibrary::get(const std::string &name) {
    auto it = s_Meshes.find(name);
    if (it != s_Meshes.end()) return it->second;
    return nullptr;
}
void MeshLibrary::add(const std::string& name, std::shared_ptr<MeshComp> mesh) {
    s_Meshes[name] = std::move(mesh);
}
glm::mat4 aiMat4ToGlm(const aiMatrix4x4& aiMat) {
    return glm::mat4(
        aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
        aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
        aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
        aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
    );
}
struct BoundingBox {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);

    void expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    [[nodiscard]] glm::vec3 size() const { return max - min; }
    [[nodiscard]] float maxDimension() const {
        glm::vec3 s = size();
        return glm::max(s.x, glm::max(s.y, s.z));
    }
};

Mesh processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i{0}; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex{};
        glm::vec3 pos = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };
        pos = glm::vec3(transform * glm::vec4(pos, 1.0f));
        vertex.Position = pos;

        if(mesh->HasNormals()) {
            glm::vec3 normal = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            };
            normal = glm::normalize(glm::mat3(transform) * normal);
            vertex.Normal = normal;
        } else {
            vertex.Normal = glm::vec3(0.0f);
        }

        if(mesh->mTextureCoords[0]) {
            vertex.TexCoords = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }
        else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i{0}; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j{0}; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    return Mesh(indices, vertices);
}

void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes,
                 const glm::mat4& parentTransform, BoundingBox& bbox)
{
    glm::mat4 nodeTransform = aiMat4ToGlm(node->mTransformation);
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh processed = processMesh(mesh, scene, globalTransform);

        for (auto& v : processed.vertices) {
            bbox.expand(v.Position);
        }

        meshes.push_back(std::move(processed));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes, globalTransform, bbox);
    }
}


void MeshLibrary::load(const std::string &name, const std::string& path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    std::vector<Mesh> meshes;
    BoundingBox bbox;
    processNode(scene->mRootNode, scene, meshes, glm::mat4(1.0f), bbox);

    float scaleFactor = 1.0f / bbox.maxDimension();
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));

    //Apply scaling to all meshes
    for (auto& mesh : meshes) {
        for (auto& v : mesh.vertices) {
            v.Position = glm::vec3(scaleMat * glm::vec4(v.Position, 1.0f));
            v.Normal = glm::normalize(glm::mat3(scaleMat) * v.Normal);
        }
    }

    s_Meshes[name] = std::make_shared<MeshComp>(meshes);
    std::cout << "Loaded model '" << name << "' with " << meshes.size() << " meshes\n";
}


std::shared_ptr<MeshComp> MeshLibrary::createCube() {
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
    return std::make_shared<MeshComp>(std::vector<Mesh>{mesh});
}

std::shared_ptr<MeshComp> MeshLibrary::createSkybox() {
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

    return std::make_shared<MeshComp>(std::vector<Mesh>{skyboxMesh});
}


std::shared_ptr<MeshComp> MeshLibrary::createSphere(float radius, unsigned int sectorCount, unsigned int stackCount) {
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

    return std::make_shared<MeshComp>(std::vector<Mesh>{sphere});
}

void MeshLibrary::shutdown() {
    s_Meshes.clear();
}



