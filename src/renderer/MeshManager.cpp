#include "ZeusEngineCore/MeshManager.h"

std::unordered_map<std::string, std::shared_ptr<IMesh>> MeshManager::s_Meshes;

std::shared_ptr<IMesh> MeshManager::Load(const std::string &name, const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices, const RendererAPI api) {
    auto it = s_Meshes.find(name);
    if(it != s_Meshes.end())
        return it->second;

    auto mesh = IMesh::Create(api);
    mesh->Init(vertices, indices);

    s_Meshes[name] = mesh;
    return mesh;
}
std::shared_ptr<IMesh> MeshManager::Get(const std::string &name) {
    auto it = s_Meshes.find(name);
    if(it != s_Meshes.end())
        return it->second;
    return nullptr;
}
