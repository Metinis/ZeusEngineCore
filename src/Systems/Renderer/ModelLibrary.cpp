#include <utility>
#include "ZeusEngineCore/ModelLibrary.h"
#include "IResourceManager.h"
#include <iostream>
#include "ZeusEngineCore/Components.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


using namespace ZEN;

namespace ZEN {
    std::unordered_map<std::string, std::shared_ptr<MeshComp>> ModelLibrary::s_Meshes;
    std::unordered_map<std::string, std::shared_ptr<MaterialComp>> ModelLibrary::s_Materials;
    IResourceManager* ModelLibrary::s_ResourceManager;
}


void ModelLibrary::init(IResourceManager* resourceManager) {
    s_Meshes["Cube"]  = createCube();
    s_Meshes["Skybox"] = createSkybox();
    s_Meshes["Sphere"] = createSphere(1.0f, 32, 16);
    s_ResourceManager = resourceManager;
}
std::shared_ptr<MeshComp> ModelLibrary::get(const std::string &name) {
    auto it = s_Meshes.find(name);
    if (it != s_Meshes.end()) return it->second;
    return nullptr;
}

std::shared_ptr<MaterialComp> ModelLibrary::getMaterial(const std::string &name) {
    auto it = s_Materials.find(name);
    if (it != s_Materials.end()) return it->second;
    return nullptr;
}

void ModelLibrary::add(const std::string& name, std::shared_ptr<MeshComp> mesh) {
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
constexpr auto processMeshPos = [](const aiVector3D& verts, const glm::mat4& transform) {
    glm::vec3 pos = {
        verts.x,
        verts.y,
        verts.z
    };
    return glm::vec3(transform * glm::vec4(pos, 1.0f)); //transformed pos
};
constexpr auto processMeshNormals = [](const aiVector3D& normals, const glm::mat4& transform) {
    glm::vec3 normal = {
        normals.x,
        normals.y,
        normals.z
    };
    return glm::normalize(glm::mat3(transform) * normal);
};
constexpr auto processMeshUVs = [](const aiVector3D& uvs) {
    glm::vec2 uv = {
        uvs.x,
        uvs.y,
    };
    return uv;
};
std::unordered_map<const aiTexture*, uint32_t> s_EmbeddedTextureCache;
constexpr auto processTexturesEmbedded = [](std::vector<uint32_t>& textureIDs, const aiScene* scene,
    const aiString& texPath) {
    unsigned int texIndex = atoi(texPath.C_Str() + 1);
    const aiTexture* tex = scene->mTextures[texIndex];
    auto it = s_EmbeddedTextureCache.find(tex);
    uint32_t texID;
    if (it != s_EmbeddedTextureCache.end()) {
        texID = it->second; // reuse
    } else {
        texID = ModelLibrary::s_ResourceManager->createTextureAssimp(*tex);
        s_EmbeddedTextureCache[tex] = texID; // cache
    }
    textureIDs.push_back(texID);
};
constexpr auto processTextureType = [](std::vector<uint32_t>& textureIDs, const aiScene* scene, const aiTextureType type,
    const aiMaterial* material) {
    uint32_t count = material->GetTextureCount(type);
    for(uint32_t i{0}; i < count; ++i) {
        aiString texPath;
        material->GetTexture(type, i, &texPath);
        if (texPath.length > 0 && texPath.C_Str()[0] == '*') {
            processTexturesEmbedded(textureIDs, scene, texPath);
        }
        else if(texPath.length > 0) {
            //load external file
        }
    }
};

entt::entity processMesh(entt::entity entity, aiMesh* mesh, const aiScene* scene, const glm::mat4& transform,
    entt::registry& registry) {
    MeshComp meshComp{};
    MaterialComp materialComp{.shaderID = 1};

    for (uint32_t i{0}; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex{};
        vertex.Position = processMeshPos(mesh->mVertices[i], transform);
        vertex.Normal = glm::vec3(0.0f);
        vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        if(mesh->HasNormals()) {
            vertex.Normal = processMeshNormals(mesh->mNormals[i], transform);
        }

        if(mesh->mTextureCoords[0]) {
            vertex.TexCoords = processMeshUVs(mesh->mTextureCoords[0][i]);
        }

        meshComp.vertices.push_back(vertex);
    }

    for (uint32_t i{0}; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j{0}; j < face.mNumIndices; ++j)
            meshComp.indices.push_back(face.mIndices[j]);
    }
    if(mesh->mMaterialIndex >= 0)
    {
        const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        processTextureType(materialComp.textureIDs, scene, aiTextureType_DIFFUSE, material);
        processTextureType(materialComp.specularTexIDs, scene, aiTextureType_SPECULAR, material);
    }


    //add new entity to registry, add mesh component and material component,
    //return it to assign trannsform and parent in processNode
    registry.emplace<MeshComp>(entity, meshComp);
    registry.emplace<MaterialComp>(entity, materialComp);
    return entity;

}

void processNode(aiNode* node, const aiScene* scene, glm::mat4& parentTransform, entt::entity parent,
    entt::registry& registry) {
    glm::mat4 nodeTransform = aiMat4ToGlm(node->mTransformation);
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        entt::entity entity = registry.create();
        processMesh(entity, mesh, scene, globalTransform, registry);
        //set parent
        if(parent != entt::null)
            registry.emplace<ParentComp>(entity, ParentComp{.parent = parent});

        registry.emplace<TransformComp>(entity, TransformComp{});
        registry.emplace<TagComp>(entity, TagComp{.tag = mesh->mName.data});
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, globalTransform, parent, registry);
    }
}


void ModelLibrary::load(const std::string &name, const std::string& path, entt::registry& registry) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_EmbedTextures);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    glm::mat4 parentTransform(1.0f);
    entt::entity parent = registry.create();
    registry.emplace<TransformComp>(parent);
    registry.emplace<TagComp>(parent, TagComp{.tag = name});
    processNode(scene->mRootNode, scene, parentTransform, parent, registry);
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

void ModelLibrary::shutdown() {
    s_Meshes.clear();
}



