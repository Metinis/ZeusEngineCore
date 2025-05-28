#include "ZeusEngineCore/IMesh.h"

#include "OpenGL/GLMesh.h"
#include "Vulkan/VKMesh.h"

std::shared_ptr<IMesh> IMesh::Create(RendererAPI api) {
    switch(api) {
        case RendererAPI::OpenGL: return std::make_shared<GLMesh>();
        case RendererAPI::Vulkan: return std::make_shared<VKMesh>();
        default: return nullptr;
    }
}
