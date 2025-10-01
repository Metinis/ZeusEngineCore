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
    private:
        void processTexturesEmbedded(std::vector<uint32_t>& textureIDs,
                                 const aiScene* scene, const aiString& texPath);

        void processTextureType(std::vector<uint32_t>& textureIDs,
                                const aiScene* scene, aiTextureType type,
                                const aiMaterial* material);

        void processAiMesh(Entity& entity, aiMesh* mesh,
                           const aiScene* scene, const glm::mat4& transform);

        void processNode(aiNode* node, const aiScene* scene,
                         const glm::mat4& parentTransform, Entity& parent);

        Scene* m_Scene{};
        ModelLibrary* m_ModelLibrary{};
        IResourceManager* m_ResourceManager{};
        std::unordered_map<const aiTexture*, uint32_t> m_EmbeddedTextureCache{};
    };
}



