#include "ZeusEngineCore/IMesh.h"

#include "OpenGL/Mesh.h"
#include "Vulkan/Mesh.h"

using namespace ZEN;

std::shared_ptr<IMesh> IMesh::Create(RendererAPI api, VKAPI::APIRenderer* apiRenderer) {
    switch(api) {
        case RendererAPI::OpenGL: return std::make_shared<OGLAPI::Mesh>();
        case RendererAPI::Vulkan: return std::make_shared<VKAPI::Mesh>(apiRenderer);
        default: return nullptr;
    }
}
