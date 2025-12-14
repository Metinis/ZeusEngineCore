#pragma once
#include <glm/fwd.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace ZEN {
    class Scene;
    class IResourceManager;
    class Entity;
    class AssetLibrary;

    class ModelImporter {
    public:
        explicit ModelImporter(Scene* scene, IResourceManager* resourceManager);
        void loadModel(const std::string &name, const std::string& path);
        void loadTexture(const std::string &name, const std::string& path);
    private:
        void processTexturesEmbedded(std::string& textureID,
                                 const aiScene* aiscene, const aiString& texPath);

        void processTextureType(std::string& textureIDs,
                                const aiScene* aiscene, aiTextureType type,
                                const aiMaterial* aimaterial);

        void processAiMesh(Entity& entity, aiMesh* mesh,
                           const aiScene* aiscene, const glm::mat4& transform);

        void processNode(aiNode* ainode, const aiScene* aiscene,
                         const glm::mat4& parentTransform, Entity& parent);

        Scene* m_Scene{};
        IResourceManager* m_ResourceManager{};
        std::unordered_map<const aiTexture*, uint32_t> m_EmbeddedTextureCache{};
        std::unordered_map<const char*, uint32_t> m_ExternalTextureCache{};
    };
}



