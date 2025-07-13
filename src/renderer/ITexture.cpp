#include "ZeusEngineCore/ITexture.h"
#include "Vulkan/Texture.h"
#include "OpenGL/Texture.h"

using namespace ZEN;

std::shared_ptr<ITexture> ITexture::Create(RendererAPI api) {
    switch (api) {
        case RendererAPI::OpenGL: return std::make_shared<OGLAPI::Texture>();
        case RendererAPI::Vulkan: return std::make_shared<VKAPI::Texture>();
    default: return nullptr;
    }
}