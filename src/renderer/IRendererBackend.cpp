#include "ZeusEngineCore/IRendererBackend.h"
#include "ZeusEngineCore/IRendererAPI.h"
#include "Vulkan/Backend/APIBackend.h"
#include "OpenGL/APIBackend.h"

using namespace ZEN;
std::unique_ptr <IRendererBackend> IRendererBackend::Create(eRendererAPI api, WindowHandle handle) {
    switch(api) {
        case eRendererAPI::OpenGL: return std::make_unique<OGLAPI::APIBackend>();
        case eRendererAPI::Vulkan: return std::make_unique<VKAPI::APIBackend>(handle);
        default: return nullptr;
    }
}
