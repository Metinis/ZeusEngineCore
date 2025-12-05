#pragma once
#include "Vertex.h"

namespace ZEN {
    //cant remove these
    const std::unordered_set<std::string> defaultMeshes = {
        "Cube", "Sphere", "Capsule"
    };
    const std::unordered_set<std::string> defaultMaterials = {
        "Default"
    };
    struct Material {
        uint32_t shaderID{0};
        uint32_t textureID{0};
        uint32_t metallicTexID{0};
        uint32_t roughnessTexID{0};
        uint32_t normalTexID{0};
        uint32_t aoTexID{0};
        glm::vec3 albedo{1.0f, 1.0f, 1.0f};
        float metallic{1.0f};
        float roughness{1.0f};
        float ao{1.0f};
        bool metal{false};
        bool useAlbedo{false};
        bool useMetallic{false};
        bool useRoughness{false};
        bool useNormal{false};
        bool useAO{false};
    };
    struct MeshData {
        std::vector<uint32_t> indices{};
        std::vector<Vertex> vertices{};
    };
    struct MeshDrawable {
        uint32_t drawableID{}; //id from resource manager
        size_t indexCount{};
        int instanceCount{1};
    };
    class IResourceManager;
    class EventDispatcher;

    class ModelLibrary {
    public:
        explicit ModelLibrary(IResourceManager* resourceManager, const std::string& resourceRoot);

        void addMeshData(const std::string& name, std::unique_ptr<MeshData> mesh);
        void addMeshData(const std::string& name, const MeshData& mesh);
        MeshData* getMeshData(const std::string& name);
        const std::unordered_map<std::string, std::unique_ptr<MeshData>>& getAllMeshData() {
            return m_MeshData;
        }
        MeshDrawable* getMeshDrawable(const std::string& name);
        const std::unordered_map<std::string, std::unique_ptr<MeshDrawable>>& getAllMeshDrawables() {
            return m_MeshDrawables;
        }
        void addMeshDrawable(const std::string &name, const MeshDrawable& drawable);
        void addMeshDrawable(const std::string &name, std::unique_ptr<MeshDrawable> drawable);

        bool hasDrawable(const std::string &name) const { return m_MeshDrawables.contains(name); }

        void createAndAddDrawable(const std::string &name, const MeshData& data);

        void removeMeshData(const std::string& name);
        void removeMeshDrawable(const std::string& name);

        void addMaterial(const std::string& name, std::unique_ptr<Material> material);
        void addMaterial(const std::string& name, const Material& material);
        Material* getMaterial(const std::string& name);
        const std::unordered_map<std::string, std::unique_ptr<Material>>& getAllMaterials() {
            return m_Materials;
        }
        void removeMaterial(const std::string& name);

        void addTexture(const std::string& name, uint32_t texID);
        uint32_t getTexture(const std::string& name);
        const std::unordered_map<std::string, uint32_t>& getAllTextures() {
            return m_Textures;
        }
        void removeTexture(const std::string& name);


    private:
        std::unordered_map<std::string, std::unique_ptr<MeshData>> m_MeshData;
        std::unordered_map<std::string, std::unique_ptr<MeshDrawable>> m_MeshDrawables;
        std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;
        std::unordered_map<std::string, uint32_t> m_Textures; //texture IDs from resource manager
        // Internal generators
        std::unique_ptr<MeshData> createCube();
        std::unique_ptr<MeshData> createQuad();
        std::unique_ptr<MeshData> createSkybox();
        std::unique_ptr<Material> createDefaultMaterial(const std::string& vertPath, const std::string& fragPath,
            const std::string& geoPath);
        //static std::shared_ptr<MeshComp> createPlane();
        std::unique_ptr<MeshData> createSphere(float radius, unsigned int sectorCount, unsigned int stackCount);
        IResourceManager* m_ResourceManager{};

    };
}


