#include "ZeusEngineCore/ITexture.h"
#include "Vulkan/Texture.h"
#include "OpenGL/Texture.h"
#include "OpenGL/APIBackend.h"
#include "OpenGL/APIRenderer.h"
#include "Vulkan/Backend/APIBackend.h"
#include "Vulkan/Backend/APIRenderer.h"

using namespace ZEN;

std::shared_ptr<ITexture> ITexture::Create(IRendererBackend* apiBackend,
                                           IRendererAPI* apiRenderer,
                                           const std::string& filepath) {
    switch(apiBackend->GetAPI()) {
        case eRendererAPI::OpenGL:{
            auto backendAPI = dynamic_cast<OGLAPI::APIBackend*>(apiBackend);
            auto rendererAPI = dynamic_cast<OGLAPI::APIRenderer*>(apiRenderer);
            OGLAPI::TextureInfo info = backendAPI->GetTextureInfo();
            info.apiRenderer = rendererAPI;
            info.filepath = filepath;
            return std::make_shared<OGLAPI::Texture>(info);
        }
        case eRendererAPI::Vulkan:
        {
            auto backendAPI = dynamic_cast<VKAPI::APIBackend*>(apiBackend);
            auto rendererAPI = dynamic_cast<VKAPI::APIRenderer*>(apiRenderer);
            VKAPI::TextureInfo info = backendAPI->GetTextureInfo();
            info.apiRenderer = rendererAPI;
            info.filepath = filepath;
            return std::make_shared<VKAPI::Texture>(info);
        }
        default: return nullptr;
    }
}