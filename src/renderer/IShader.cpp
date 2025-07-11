#include "ZeusEngineCore/IShader.h"

#include "OpenGL/GLShader.h"
#include "Vulkan/VKShaderPipeline.h"

std::shared_ptr<IShader> IShader::Create(RendererAPI api) {
    switch(api) {
        case RendererAPI::OpenGL: return std::make_shared<GLShader>();
        case RendererAPI::Vulkan: return std::make_shared<VKShaderPipeline>();
        default: return nullptr;
    }
}
