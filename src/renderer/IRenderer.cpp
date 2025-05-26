#include "ZeusEngineCore/IRenderer.h"
#include "ZeusEngineCore/RendererAPI.h"
#include "OpenGL/OpenGLRenderer.h"
#include "Vulkan/VulkanRenderer.h"

std::unique_ptr<IRenderer> IRenderer::Create(RendererAPI api) {
    switch (api) {
        case RendererAPI::OpenGL: return std::make_unique<OpenGLRenderer>();
        case RendererAPI::Vulkan: return std::make_unique<VulkanRenderer>();
        default: return nullptr;
    }
}

