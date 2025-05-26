#include "ZeusEngineCore/IMesh.h"

#include "OpenGL/OpenGLMesh.h"
#include "Vulkan/VulkanMesh.h"

std::shared_ptr<IMesh> IMesh::Create(RendererAPI api) {
    switch(api) {
        case RendererAPI::OpenGL: return std::make_shared<OpenGLMesh>();
        //case RendererAPI::Vulkan: return std::make_shared<VulkanMesh>();
        default: return nullptr;
    }
}
