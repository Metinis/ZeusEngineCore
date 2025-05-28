#include "ZeusEngineCore/IRenderer.h"
#include "ZeusEngineCore/RendererAPI.h"
#include "OpenGL/GLRenderer.h"
#include "Vulkan/VKRenderer.h"

std::unique_ptr<IRenderer> IRenderer::Create(RendererAPI api) {
    switch (api) {
        case RendererAPI::OpenGL: return std::make_unique<GLRenderer>();
        case RendererAPI::Vulkan: return std::make_unique<VKRenderer>();
        default: return nullptr;
    }
}

