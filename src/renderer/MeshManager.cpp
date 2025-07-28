#include "ZeusEngineCore/MeshManager.h"
#include "ZeusEngineCore/IMesh.h"
using namespace ZEN;

MeshManager::MeshManager(IRendererBackend* backendAPI, IRendererAPI* rendererAPI)
: m_BackendAPI(backendAPI),
m_RendererAPI(rendererAPI)
{

}

std::shared_ptr<IMesh> MeshManager::Load(const std::string &name, const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices) {
    auto it = m_Meshes.find(name);
    if(it != m_Meshes.end())
        return it->second;

    auto mesh = IMesh::Create(m_BackendAPI, m_RendererAPI, vertices, indices);
    m_Meshes[name] = mesh;
    return mesh;
}
std::shared_ptr<IMesh> MeshManager::Get(const std::string &name) {
    auto it = m_Meshes.find(name);
    if(it != m_Meshes.end())
        return it->second;
    return nullptr;
}


