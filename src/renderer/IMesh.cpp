#include "ZeusEngineCore/IMesh.h"
#include "Vulkan/Backend/APIBackend.h"
#include "Vulkan/Backend/APIRenderer.h"
#include "OpenGL/APIBackend.h"
#include "OpenGL/APIRenderer.h"
#include "OpenGL/Mesh.h"
#include "Vulkan/Mesh.h"
#include "ZeusEngineCore/Vertex.h"

using namespace ZEN;

std::shared_ptr<IMesh> IMesh::Create(IRendererBackend* apiBackend,
                                     IRendererAPI* apiRenderer,
                                     const std::vector<Vertex>& vertices,
                                     const std::vector<uint32_t>& indices) {
    switch(apiBackend->GetAPI()) {
        case eRendererAPI::OpenGL:{
            auto backendAPI = dynamic_cast<OGLAPI::APIBackend*>(apiBackend);
            auto rendererAPI = dynamic_cast<OGLAPI::APIRenderer*>(apiRenderer);
            OGLAPI::MeshInfo info = backendAPI->GetMeshInfo();
            info.apiRenderer = rendererAPI;
            info.vertices = vertices;
            info.indices = indices;
            return std::make_shared<OGLAPI::Mesh>(info);
        }
        case eRendererAPI::Vulkan:
        {
            auto backendAPI = dynamic_cast<VKAPI::APIBackend*>(apiBackend);
            auto rendererAPI = dynamic_cast<VKAPI::APIRenderer*>(apiRenderer);
            VKAPI::MeshInfo info = backendAPI->GetMeshInfo();
            info.apiRenderer = rendererAPI;
            info.vertices = vertices;
            info.indices = indices;
            return std::make_shared<VKAPI::Mesh>(info);
        }
        default: return nullptr;
    }
}
