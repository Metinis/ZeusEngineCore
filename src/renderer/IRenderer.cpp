#include "ZeusEngineCore/IRenderer.h"
#include "ZeusEngineCore/RendererAPI.h"
#include "OpenGL/Renderer.h"
#include "Vulkan/Renderer.h"

using namespace ZEN;

std::unique_ptr<IRenderer> IRenderer::Create(RendererAPI api) {
    switch (api) {
        case RendererAPI::OpenGL: return std::make_unique<OGLAPI::Renderer>();
        case RendererAPI::Vulkan: return std::make_unique<VKAPI::Renderer>();
        default: return nullptr;
    }
}

