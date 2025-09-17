#pragma once
#include <memory>
#include <unordered_map>
#include <string>

namespace ZEN {
    struct MeshComp;

    class MeshLibrary {
    public:
        static void init();
        static void shutdown();

        static std::shared_ptr<MeshComp> get(const std::string& name);
        static const std::unordered_map<std::string, std::shared_ptr<MeshComp>>& getAll() {
            return s_Meshes;
        }
        static void add(const std::string& name, std::shared_ptr<MeshComp> mesh);

    private:
        static std::unordered_map<std::string, std::shared_ptr<MeshComp>> s_Meshes;

        // Internal generators
        static std::shared_ptr<MeshComp> createCube();
        static std::shared_ptr<MeshComp> createSkybox();
        //static std::shared_ptr<MeshComp> createPlane();
        //static std::shared_ptr<MeshComp> createSphere();
    };
}


