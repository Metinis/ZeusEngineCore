#pragma once
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

        void addMesh(const std::string& name, std::shared_ptr<MeshComp> mesh);
        void addMesh(const std::string& name, const MeshComp& mesh);
        std::shared_ptr<MeshComp> getMesh(const std::string& name);
        const std::unordered_map<std::string, std::shared_ptr<MeshComp>>& getAllMeshes() {
            return m_Meshes;
        }

        void addMaterial(const std::string& name, std::shared_ptr<MaterialComp> material);
        void addMaterial(const std::string& name, const MaterialComp& material);
        std::shared_ptr<MaterialComp> getMaterial(const std::string& name);
        const std::unordered_map<std::string, std::shared_ptr<MaterialComp>>& getAllMaterials() {
            return m_Materials;
        }


    private:
        std::unordered_map<std::string, std::shared_ptr<MeshComp>> m_Meshes;
        std::unordered_map<std::string, std::shared_ptr<MaterialComp>> m_Materials;
        // Internal generators
        std::shared_ptr<MeshComp> createCube();
        std::shared_ptr<MeshComp> createSkybox();
        //static std::shared_ptr<MeshComp> createPlane();
        std::shared_ptr<MeshComp> createSphere(float radius, unsigned int sectorCount, unsigned int stackCount);
        IResourceManager* m_ResourceManager;

    };
}


