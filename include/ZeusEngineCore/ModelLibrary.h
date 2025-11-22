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
        bool metal{};
        bool useAlbedo;
        bool useMetallic;
        bool useRoughness;
        bool useNormal;
        bool useAO;
    };
    struct Mesh {
        std::vector<uint32_t> indices{};
        std::vector<Vertex> vertices{};
    };
    class IResourceManager;
    class EventDispatcher;

    class ModelLibrary {
    public:
        explicit ModelLibrary(EventDispatcher* dispatcher, IResourceManager* resourceManager,
            const std::string& resourceRoot);

        void addMesh(const std::string& name, std::unique_ptr<Mesh> mesh);
        void addMesh(const std::string& name, const Mesh& mesh);
        Mesh* getMesh(const std::string& name);
        const std::unordered_map<std::string, std::unique_ptr<Mesh>>& getAllMeshes() {
            return m_Meshes;
        }
        void removeMesh(const std::string& name);

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
        std::unordered_map<std::string, std::unique_ptr<Mesh>> m_Meshes;
        std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;
        std::unordered_map<std::string, uint32_t> m_Textures; //texture IDs from resource manager
        // Internal generators
        std::unique_ptr<Mesh> createCube();
        std::unique_ptr<Mesh> createQuad();
        std::unique_ptr<Mesh> createSkybox();
        std::unique_ptr<Material> createDefaultMaterial(const std::string& vertPath, const std::string& fragPath,
            const std::string& geoPath);
        //static std::shared_ptr<MeshComp> createPlane();
        std::unique_ptr<Mesh> createSphere(float radius, unsigned int sectorCount, unsigned int stackCount);
        EventDispatcher* m_Dispatcher{};
        IResourceManager* m_ResourceManager{};

    };
}


