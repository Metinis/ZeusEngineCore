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
        static void init(IResourceManager* resourceManager);
        static void shutdown();

        static std::shared_ptr<MeshComp> get(const std::string& name);
        static const std::unordered_map<std::string, std::shared_ptr<MeshComp>>& getAll() {
            return s_Meshes;
        }
        static std::shared_ptr<MaterialComp> getMaterial(const std::string& name);
        static void add(const std::string& name, std::shared_ptr<MeshComp> mesh);
        static void load(const std::string& name, const std::string& path);
        static IResourceManager* s_ResourceManager;

    private:
        static std::unordered_map<std::string, std::shared_ptr<MeshComp>> s_Meshes;
        static std::unordered_map<std::string, std::shared_ptr<MaterialComp>> s_Materials;
        // Internal generators
        static std::shared_ptr<MeshComp> createCube();
        static std::shared_ptr<MeshComp> createSkybox();
        //static std::shared_ptr<MeshComp> createPlane();
        static std::shared_ptr<MeshComp> createSphere(float radius, unsigned int sectorCount, unsigned int stackCount);


    };
}


