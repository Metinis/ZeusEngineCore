#include "ZeusEngineCore/IRendererAPI.h"
#include "OpenGL/APIRenderer.h"
#include "Vulkan/Backend/APIRenderer.h"
#include "OpenGL/APIBackend.h"
#include "ZeusEngineCore/IRendererBackend.h"

using namespace ZEN;
std::unique_ptr<IRendererAPI> IRendererAPI::Create(eRendererAPI api, IRendererBackend* apiBackend) {
    switch(api) {
        case eRendererAPI::OpenGL: return std::make_unique<OGLAPI::APIRenderer>(dynamic_cast<OGLAPI::APIBackend*>(apiBackend));
        case eRendererAPI::Vulkan: return std::make_unique<VKAPI::APIRenderer>(dynamic_cast<VKAPI::APIBackend*>(apiBackend));
        default: return nullptr;
    }
}
