#pragma once
#include <string>
#include <entt/entt.hpp>
#include <glm/fwd.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace ZEN {
    class Scene;
    class IResourceManager;

    class ModelImporter {
    public:
        explicit ModelImporter(Scene* scene, IResourceManager* resourceManager);
        void loadModel(const std::string &name, const std::string& path);
    private:
        void processTexturesEmbedded(std::vector<uint32_t>& textureIDs,
                                 const aiScene* scene, const aiString& texPath);

        void processTextureType(std::vector<uint32_t>& textureIDs,
                                const aiScene* scene, aiTextureType type,
                                const aiMaterial* material);

        void processAiMesh(entt::entity entity, aiMesh* mesh,
                           const aiScene* scene, const glm::mat4& transform);

        void processNode(aiNode* node, const aiScene* scene,
                         const glm::mat4& parentTransform, entt::entity parent);

        Scene* m_Scene{};
        IResourceManager* m_ResourceManager{};
        std::unordered_map<const aiTexture*, uint32_t> m_EmbeddedTextureCache{};
    };
}



