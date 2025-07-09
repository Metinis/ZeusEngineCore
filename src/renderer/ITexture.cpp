#include "ZeusEngineCore/ITexture.h"
#include "Vulkan/VKTexture.h"
#include "OpenGL/GLTexture.h"

std::shared_ptr<ITexture> ITexture::Create(RendererAPI api) {
    switch (api) {
    case RendererAPI::OpenGL: return std::make_shared<GLTexture>();
    case RendererAPI::Vulkan: return std::make_shared<VKTexture>();
    default: return nullptr;
    }
}