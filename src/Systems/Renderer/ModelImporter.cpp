#include "ZeusEngineCore/ModelImporter.h"
#include <iostream>
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/Components.h>
#include <ZeusEngineCore/Scene.h>


using namespace ZEN;

ModelImporter::ModelImporter(Scene *scene, IResourceManager* resourceManager) : m_Scene(scene),
m_ResourceManager(resourceManager){

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

void ModelImporter::processTexturesEmbedded(std::vector<uint32_t>& textureIDs,
                                            const aiScene* scene, const aiString& texPath) {
    unsigned int texIndex = std::atoi(texPath.C_Str() + 1);
    const aiTexture* tex = scene->mTextures[texIndex];

    auto it = m_EmbeddedTextureCache.find(tex);
    uint32_t texID;
    if (it != m_EmbeddedTextureCache.end()) {
        texID = it->second; // reuse
    } else {
        texID = m_ResourceManager->createTextureAssimp(*tex);
        m_EmbeddedTextureCache[tex] = texID; // cache
    }
    textureIDs.push_back(texID);
}

void ModelImporter::processTextureType(std::vector<uint32_t>& textureIDs,
                                       const aiScene* scene, aiTextureType type,
                                       const aiMaterial* material) {
    uint32_t count = material->GetTextureCount(type);
    for (uint32_t i{0}; i < count; ++i) {
        aiString texPath;
        material->GetTexture(type, i, &texPath);
        if (texPath.length > 0 && texPath.C_Str()[0] == '*') {
            processTexturesEmbedded(textureIDs, scene, texPath);
        } else if (texPath.length > 0) {
            // TODO: load external file
        }
    }
}

void ModelImporter::processAiMesh(Entity& entity, aiMesh* mesh,
                                  const aiScene* scene, const glm::mat4& transform) {
    MeshComp meshComp{};
    MaterialComp materialComp{.shaderID = 1};

    for (uint32_t i{0}; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};
        vertex.Position = processMeshPos(mesh->mVertices[i], transform);
        vertex.Normal   = glm::vec3(0.0f);
        vertex.TexCoords= glm::vec2(0.0f, 0.0f);

        if (mesh->HasNormals())
            vertex.Normal = processMeshNormals(mesh->mNormals[i], transform);

        if (mesh->mTextureCoords[0])
            vertex.TexCoords = processMeshUVs(mesh->mTextureCoords[0][i]);

        meshComp.vertices.push_back(vertex);
    }

    for (uint32_t i{0}; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j{0}; j < face.mNumIndices; ++j)
            meshComp.indices.push_back(face.mIndices[j]);
    }

    if (mesh->mMaterialIndex >= 0) {
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        processTextureType(materialComp.textureIDs, scene, aiTextureType_DIFFUSE, material);
        processTextureType(materialComp.specularTexIDs, scene, aiTextureType_SPECULAR, material);
    }

    entity.addComponent<MeshComp>(meshComp);
    entity.addComponent<MaterialComp>(materialComp);
}

void ModelImporter::processNode(aiNode* node, const aiScene* scene,
                                const glm::mat4& parentTransform, Entity& parent) {
    glm::mat4 nodeTransform = aiMat4ToGlm(node->mTransformation);
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Entity entity = m_Scene->createEntity(mesh->mName.C_Str());
        processAiMesh(entity, mesh, scene, globalTransform);

        entity.addComponent<ParentComp>(parent);

    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, globalTransform, parent);
    }
}

void ModelImporter::loadModel(const std::string &name, const std::string &path) {
    Assimp::Importer import;
    import.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f); // cm → m
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_EmbedTextures);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    glm::mat4 parentTransform(1.0f);
    Entity parent = m_Scene->createEntity(name);
    processNode(scene->mRootNode, scene, parentTransform, parent);
}
