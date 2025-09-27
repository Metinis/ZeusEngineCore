#pragma once
#include <entt.hpp>
#include <memory>
#include <unordered_map>
#include <string>


namespace ZEN {
    struct MeshComp;
    struct MaterialComp;
    class IResourceManager;

    class ModelLibrary {
    public:
        explicit ModelLibrary(IResourceManager* resourceManager);

        std::shared_ptr<MeshComp> getMesh(const std::string& name);
        const std::unordered_map<std::string, std::shared_ptr<MeshComp>>& getAllMeshes() {
            return s_Meshes;
        }
        std::shared_ptr<MaterialComp> getMaterial(const std::string& name);
        void addMesh(const std::string& name, std::shared_ptr<MeshComp> mesh);
        void loadModel(const std::string& name, const std::string& path, entt::registry& registry);


    private:
        std::unordered_map<std::string, std::shared_ptr<MeshComp>> s_Meshes;
        std::unordered_map<std::string, std::shared_ptr<MaterialComp>> s_Materials;
        // Internal generators
        std::shared_ptr<MeshComp> createCube();
        std::shared_ptr<MeshComp> createSkybox();
        //static std::shared_ptr<MeshComp> createPlane();
        std::shared_ptr<MeshComp> createSphere(float radius, unsigned int sectorCount, unsigned int stackCount);
        IResourceManager* m_ResourceManager;

    };
}


