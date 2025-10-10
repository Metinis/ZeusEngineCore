#pragma once
#include <unordered_map>
#include <glm/fwd.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <vector>

namespace ZEN {
    class Scene;
    class IResourceManager;
    class Entity;
    class ModelLibrary;

    class ModelImporter {
    public:
        explicit ModelImporter(Scene* scene, IResourceManager* resourceManager, ModelLibrary* modelLibrary);
        void loadModel(const std::string &name, const std::string& path);
        void loadTexture(const std::string &name, const std::string& path);
    private:
        void processTexturesEmbedded(std::vector<uint32_t>& textureIDs,
                                 const aiScene* aiscene, const aiString& texPath);

        void processTextureType(std::vector<uint32_t>& textureIDs,
                                const aiScene* aiscene, aiTextureType type,
                                const aiMaterial* aimaterial);

        void processAiMesh(Entity& entity, aiMesh* mesh,
                           const aiScene* aiscene, const glm::mat4& transform);

        void processNode(aiNode* ainode, const aiScene* aiscene,
                         const glm::mat4& parentTransform, Entity& parent);

        Scene* m_Scene{};
        IResourceManager* m_ResourceManager{};
        ModelLibrary* m_ModelLibrary{};
        std::unordered_map<const aiTexture*, uint32_t> m_EmbeddedTextureCache{};
        std::unordered_map<const char*, uint32_t> m_ExternalTextureCache{};
    };
}



