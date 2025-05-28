#include "ZeusEngineCore/IShader.h"

#include "OpenGL/GLShader.h"
#include "Vulkan/VKShader.h"

std::shared_ptr<IShader> IShader::Create(RendererAPI api) {
    switch(api) {
        case RendererAPI::OpenGL: return std::make_shared<GLShader>();
        case RendererAPI::Vulkan: return std::make_shared<VKShader>();
        default: return nullptr;
    }
}
