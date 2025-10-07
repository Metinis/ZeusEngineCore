#include "ZeusEngineCore/ModelImporter.h"
#include <iostream>
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/Components.h>
#include <ZeusEngineCore/ModelLibrary.h>
#include <ZeusEngineCore/Scene.h>


using namespace ZEN;

ModelImporter::ModelImporter(Scene* scene, IResourceManager* resourceManager, ModelLibrary* modelLibrary) : m_Scene(scene),
m_ResourceManager(resourceManager), m_ModelLibrary(modelLibrary){

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
                                            const aiScene* aiscene, const aiString& texPath) {
    unsigned int texIndex = std::atoi(texPath.C_Str() + 1);
    const aiTexture* tex = aiscene->mTextures[texIndex];

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
                                       const aiScene* aiscene, aiTextureType type,
                                       const aiMaterial* aimaterial) {
    uint32_t count = aimaterial->GetTextureCount(type);
    for (uint32_t i{0}; i < count; ++i) {
        aiString texPath;
        aimaterial->GetTexture(type, i, &texPath);
        if (texPath.length > 0 && texPath.C_Str()[0] == '*') {
            processTexturesEmbedded(textureIDs, aiscene, texPath);
        } else if (texPath.length > 0) {
            std::cout<<"Trying to load external file!\n";
            uint32_t texID;
            auto it = m_ExternalTextureCache.find(texPath.C_Str());
            if (it != m_ExternalTextureCache.end()) {
                texID = it->second;
            } else {
                texID = m_ResourceManager->createTexture(texPath.C_Str());
                m_ExternalTextureCache[texPath.C_Str()] = texID;
            }
            textureIDs.push_back(texID);
        }
        else {
            std::cout<<"Warning! No texture path found!\n";
        }
    }
}

void ModelImporter::processAiMesh(Entity& entity, aiMesh* aimesh,
                                  const aiScene* aiscene, const glm::mat4& transform) {
    Mesh mesh{};
    Material material{.shaderID = m_ModelLibrary->getMaterial("Default")->shaderID};

    for (uint32_t i{0}; i < aimesh->mNumVertices; ++i) {
        Vertex vertex{};
        vertex.Position = processMeshPos(aimesh->mVertices[i], transform);
        vertex.Normal   = glm::vec3(0.0f);
        vertex.TexCoords= glm::vec2(0.0f, 0.0f);

        if (aimesh->HasNormals())
            vertex.Normal = processMeshNormals(aimesh->mNormals[i], transform);

        if (aimesh->mTextureCoords[0])
            vertex.TexCoords = processMeshUVs(aimesh->mTextureCoords[0][i]);

        mesh.vertices.push_back(vertex);
    }

    for (uint32_t i{0}; i < aimesh->mNumFaces; ++i) {
        aiFace face = aimesh->mFaces[i];
        for (uint32_t j{0}; j < face.mNumIndices; ++j)
            mesh.indices.push_back(face.mIndices[j]);
    }

    if (aimesh->mMaterialIndex >= 0) {
        const aiMaterial* aiMaterial = aiscene->mMaterials[aimesh->mMaterialIndex];
        processTextureType(material.textureIDs, aiscene, aiTextureType_DIFFUSE, aiMaterial);
        if(material.textureIDs.empty()) {
            std::cout<<"Warning! No Textures found for mesh: "<< aimesh->mName.C_Str() << "\n";
        }
        processTextureType(material.specularTexIDs, aiscene, aiTextureType_SPECULAR, aiMaterial);
    }
    m_ModelLibrary->addMesh(aimesh->mName.C_Str(), mesh);
    m_ModelLibrary->addMaterial(aiscene->mMaterials[aimesh->mMaterialIndex]->GetName().C_Str(), material);

    entity.addComponent<MeshComp>(MeshComp{.name = aimesh->mName.C_Str()});
    entity.addComponent<MaterialComp>(MaterialComp{.name = aiscene->mMaterials[aimesh->mMaterialIndex]->GetName().C_Str()});

}

void ModelImporter::processNode(aiNode* ainode, const aiScene* aiscene,
                                const glm::mat4& parentTransform, Entity& parent) {
    glm::mat4 nodeTransform = aiMat4ToGlm(ainode->mTransformation);
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < ainode->mNumMeshes; i++) {
        aiMesh* mesh = aiscene->mMeshes[ainode->mMeshes[i]];
        Entity entity = m_Scene->createEntity(mesh->mName.C_Str());
        processAiMesh(entity, mesh, aiscene, globalTransform);

        entity.addComponent<ParentComp>(parent);

    }

    for (unsigned int i = 0; i < ainode->mNumChildren; i++) {
        processNode(ainode->mChildren[i], aiscene, globalTransform, parent);
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

void ModelImporter::loadTexture(const std::string &name, const std::string &path) {
    m_ModelLibrary->addTexture(name, m_ResourceManager->createTexture(path));
}
