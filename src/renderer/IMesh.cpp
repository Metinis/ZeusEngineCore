#include "ZeusEngineCore/IMesh.h"

#include "OpenGL/Mesh.h"
#include "Vulkan/Mesh.h"

using namespace ZEN;

std::shared_ptr<IMesh> IMesh::Create(RendererAPI api) {
    switch(api) {
        case RendererAPI::OpenGL: return std::make_shared<OGLAPI::Mesh>();
        case RendererAPI::Vulkan: return std::make_shared<VKAPI::Mesh>();
        default: return nullptr;
    }
}
