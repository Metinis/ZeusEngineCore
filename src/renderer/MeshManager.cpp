#include "ZeusEngineCore/MeshManager.h"

std::shared_ptr<IMesh> MeshManager::Load(const std::string &name, const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices, const RendererAPI api) {
    auto it = m_Meshes.find(name);
    if(it != m_Meshes.end())
        return it->second;

    auto mesh = IMesh::Create(api);
    mesh->Init(vertices, indices);

    m_Meshes[name] = mesh;
    return mesh;
}
std::shared_ptr<IMesh> MeshManager::Get(const std::string &name) {
    auto it = m_Meshes.find(name);
    if(it != m_Meshes.end())
        return it->second;
    return nullptr;
}
