#include "ZeusEngineCore/ModelImporter.h"
#include <ZeusEngineCore/Entity.h>
#include <ZeusEngineCore/Components.h>
#include <ZeusEngineCore/AssetLibrary.h>
#include <ZeusEngineCore/Scene.h>


using namespace ZEN;

ModelImporter::ModelImporter() :
m_Scene(&Application::get().getEngine()->getScene()),
m_ResourceManager(Application::get().getEngine()->getRenderer().getResourceManager()),
m_AssetLibrary(Project::getActive()->getAssetLibrary()) {

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

UUID ModelImporter::processTexturesEmbedded(const aiScene* aiscene, const aiString& texPath) {
    unsigned int texIndex = std::atoi(texPath.C_Str() + 1);
    aiTexture* tex = aiscene->mTextures[texIndex];

    auto it = m_EmbeddedTextureCache.find(tex);
    if (it != m_EmbeddedTextureCache.end()) {
        return it->second; // reuse
    } else {
        auto texData = TextureData{.type = Texture2DAssimp, .aiTex = tex};
        auto id = m_AssetLibrary->createAsset(std::move(texData));
        m_EmbeddedTextureCache[tex] = id; // cache
        return id;
    }
}

UUID ModelImporter::processTextureType(const aiScene* aiscene, aiTextureType type,
                                       const aiMaterial* aimaterial) {
    uint32_t count = aimaterial->GetTextureCount(type);
    for (uint32_t i{0}; i < count; ++i) {
        aiString texPath;
        aimaterial->GetTexture(type, i, &texPath);
        std::cout<<texPath.data<<"\n";
        if (texPath.length > 0 && texPath.C_Str()[0] == '*') {
            return processTexturesEmbedded(aiscene, texPath);
        } else if (texPath.length > 0) {
            std::cout<<"Trying to load external file!\n";
            auto it = m_ExternalTextureCache.find(texPath.C_Str());
            if (it != m_ExternalTextureCache.end()) {
                return it->second;
            } else {
                TextureData texData {
                    .path = texPath.C_Str(),
                    .type = Texture2D,
                    .dimensions = glm::vec2{},
                    .mip = false,
                    .absPath = true,
                };
                auto texID = m_AssetLibrary->createAsset(std::move(texData));
                m_ExternalTextureCache[texPath.C_Str()] = texID;
                return texID;

            }}
        else {
            std::cout<<"Warning! No texture path found!\n";
        }
    }
}
/*void processAllTextureTypes(const aiMaterial* aiMaterial) {
    for (int t = aiTextureType_NONE; t <= aiTextureType_UNKNOWN; ++t) {
        aiTextureType type = static_cast<aiTextureType>(t);
        unsigned int count = aiMaterial->GetTextureCount(type);

        std::cout<<count<<"\n";
    }
}*/
void ModelImporter::processAiMesh(Entity& entity, aiMesh* aimesh,
                                  const aiScene* aiscene, const glm::mat4& transform) {
    MeshData mesh{};
    //Material material{.shader = "Default"};
    for (uint32_t i{0}; i < aimesh->mNumVertices; ++i) {
        Vertex vertex{};
        vertex.Position = processMeshPos(aimesh->mVertices[i], transform);
        vertex.Normal   = glm::vec3(0.0f);
        vertex.TexCoords= glm::vec2(0.0f, 0.0f);
        vertex.Tangent.x = aimesh->mTangents[i].x;
        vertex.Tangent.y = aimesh->mTangents[i].y;
        vertex.Tangent.z = aimesh->mTangents[i].z;

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

    Material material = *AssetHandle<Material>(m_AssetLibrary->getDefaultMaterialID()).get();
    material.useAlbedo = true;
    //material.useAO = true;
    //material.useMetallic = true;
    //material.useNormal = true;
    //material.useRoughness = true;
    if (aimesh->mMaterialIndex >= 0) {
        const aiMaterial* aiMaterial = aiscene->mMaterials[aimesh->mMaterialIndex];
        material.texture = processTextureType(aiscene, aiTextureType_DIFFUSE, aiMaterial);
        //material.roughnessTex = processTextureType(aiscene, aiTextureType_DIFFUSE_ROUGHNESS, aiMaterial);
        //material.metallicTex = processTextureType(aiscene, aiTextureType_METALNESS, aiMaterial);
        //material.normalTex = processTextureType(aiscene, aiTextureType_NORMALS, aiMaterial);
    }
    auto matID = m_AssetLibrary->createAsset<Material>(std::move(material), aiscene->mMaterials[aimesh->mMaterialIndex]->GetName().C_Str());
    auto meshID = m_AssetLibrary->createAsset<MeshData>(std::move(mesh), aimesh->mName.C_Str());

    entity.addComponent<MeshComp>(meshID);
    entity.addComponent<MaterialComp>(matID);

}

void ModelImporter::processNode(aiNode* ainode, const aiScene* aiscene,
                                const glm::mat4& parentTransform, Entity& parent) {
    glm::mat4 nodeTransform = aiMat4ToGlm(ainode->mTransformation);
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < ainode->mNumMeshes; i++) {
        aiMesh* mesh = aiscene->mMeshes[ainode->mMeshes[i]];
        Entity entity = m_Scene->createEntity(mesh->mName.C_Str());
        processAiMesh(entity, mesh, aiscene, globalTransform);

        entity.addComponent<ParentComp>(parent.getComponent<UUIDComp>().uuid);

    }

    for (unsigned int i = 0; i < ainode->mNumChildren; i++) {
        processNode(ainode->mChildren[i], aiscene, globalTransform, parent);
    }
}

void ModelImporter::loadModel(const std::string &name, const std::string &path) {
    Assimp::Importer import;
    import.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f); // cm → m
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
        aiProcess_EmbedTextures | aiProcess_CalcTangentSpace | aiProcess_GlobalScale);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    glm::mat4 parentTransform(1.0f);
    Entity parent = m_Scene->createEntity(name);
    processNode(scene->mRootNode, scene, parentTransform, parent);
}

void ModelImporter::loadTexture(const std::string &name, const std::string &path) {
    m_AssetLibrary->createAsset(TextureData{.path = path, .absPath = true}, name);
}
