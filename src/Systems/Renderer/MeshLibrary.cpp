#include <utility>

#include "ZeusEngineCore/MeshLibrary.h"
#include "ZeusEngineCore/Components.h"

using namespace ZEN;

namespace ZEN {
    std::unordered_map<std::string, std::shared_ptr<MeshComp>> MeshLibrary::s_Meshes;
}


void MeshLibrary::init() {
    s_Meshes["Cube"]  = createCube();
    s_Meshes["Skybox"] = createSkybox();
}
std::shared_ptr<MeshComp> MeshLibrary::get(const std::string &name) {
    auto it = s_Meshes.find(name);
    if (it != s_Meshes.end()) return it->second;
    return nullptr;
}
void MeshLibrary::add(const std::string &name, std::shared_ptr<MeshComp> mesh) {
    s_Meshes[name] = std::move(mesh);
}

std::shared_ptr<MeshComp> MeshLibrary::createCube() {
    MeshComp mesh;

    mesh.vertices = {
        // Front face (z = +0.5)
        {{-0.5f,  0.5f,  0.5f}, {0,0,1}, {0.0f, 1.0f}}, // Top-left
        {{ 0.5f,  0.5f,  0.5f}, {0,0,1}, {1.0f, 1.0f}}, // Top-right
        {{ 0.5f, -0.5f,  0.5f}, {0,0,1}, {1.0f, 0.0f}}, // Bottom-right
        {{-0.5f, -0.5f,  0.5f}, {0,0,1}, {0.0f, 0.0f}}, // Bottom-left

        // Back face (z = -0.5)
        {{ 0.5f,  0.5f, -0.5f}, {0,0,-1}, {0.0f, 1.0f}}, // Top-left
        {{-0.5f,  0.5f, -0.5f}, {0,0,-1}, {1.0f, 1.0f}}, // Top-right
        {{-0.5f, -0.5f, -0.5f}, {0,0,-1}, {1.0f, 0.0f}}, // Bottom-right
        {{ 0.5f, -0.5f, -0.5f}, {0,0,-1}, {0.0f, 0.0f}}, // Bottom-left

        // Left face (x = -0.5)
        {{-0.5f,  0.5f, -0.5f}, {-1,0,0}, {0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1,0,0}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1,0,0}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1,0,0}, {0.0f, 0.0f}},

        // Right face (x = +0.5)
        {{ 0.5f,  0.5f,  0.5f}, {1,0,0}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1,0,0}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1,0,0}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1,0,0}, {0.0f, 0.0f}},

        // Top face (y = +0.5)
        {{-0.5f,  0.5f, -0.5f}, {0,1,0}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0,1,0}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0,1,0}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0,1,0}, {0.0f, 0.0f}},

        // Bottom face (y = -0.5)
        {{-0.5f, -0.5f,  0.5f}, {0,-1,0}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0,-1,0}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0,-1,0}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0,-1,0}, {0.0f, 0.0f}}
    };

    mesh.indices = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Back
        4, 5, 6, 6, 7, 4,
        // Left
        8, 9,10,10,11, 8,
        // Right
       12,13,14,14,15,12,
        // Top
       16,17,18,18,19,16,
        // Bottom
       20,21,22,22,23,20
    };
    return std::make_shared<MeshComp>(mesh);
}

std::shared_ptr<MeshComp> MeshLibrary::createSkybox() {
    MeshComp skyboxMesh{};
    skyboxMesh.vertices = {
        {{-1.0f,  1.0f, -1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, // Back
        {{-1.0f, -1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, // Front
        {{-1.0f,  1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}}, // Left
        {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}}, // Right
        {{-1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f,  1.0f}}, // Top
        {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f, -1.0f}}  // Bottom
    };

    // Cube indices
    skyboxMesh.indices = {
        0,1,2, 2,3,0,       // Back
        4,5,6, 6,7,4,       // Front
        8,9,10, 10,11,8,    // Left
        12,13,14, 14,15,12, // Right
        16,17,18, 18,19,16, // Top
        20,21,22, 22,23,20  // Bottom
    };
    return std::make_shared<MeshComp>(skyboxMesh);
}

void MeshLibrary::shutdown() {
    s_Meshes.clear();
}



