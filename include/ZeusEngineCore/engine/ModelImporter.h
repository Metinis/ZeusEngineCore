#pragma once
#include <glm/fwd.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "UUID.h"

namespace ZEN {
    class Scene;
    class IResourceManager;
    class Entity;
    class AssetLibrary;

    class ModelImporter {
    public:
        explicit ModelImporter();
        void loadModel(const std::string &name, const std::string& path);
        void loadTexture(const std::string &name, const std::string& path);
    private:
        UUID processTexturesEmbedded(const aiScene* aiscene, const aiString& texPath);

        UUID processTextureType(const aiScene* aiscene, aiTextureType type,
                                const aiMaterial* aimaterial);

        void processAiMesh(Entity& entity, aiMesh* mesh,
                           const aiScene* aiscene, const glm::mat4& transform);

        void processNode(aiNode* ainode, const aiScene* aiscene,
                         const glm::mat4& parentTransform, Entity& parent);

        Scene* m_Scene{};
        IResourceManager* m_ResourceManager{};
        std::shared_ptr<AssetLibrary> m_AssetLibrary{};
        std::unordered_map<const aiTexture*, UUID> m_EmbeddedTextureCache{};
        std::unordered_map<const char*, UUID> m_ExternalTextureCache{};
    };
}



