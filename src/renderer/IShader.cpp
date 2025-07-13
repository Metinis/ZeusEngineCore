#include "ZeusEngineCore/IShader.h"

#include "OpenGL/Shader.h"
#include "Vulkan/ShaderPipeline.h"

using namespace ZEN;

std::shared_ptr<IShader> IShader::Create(RendererAPI api) {
    switch(api) {
        case RendererAPI::OpenGL: return std::make_shared<OGLAPI::Shader>();
        case RendererAPI::Vulkan: return std::make_shared<VKAPI::ShaderPipeline>();
        default: return nullptr;
    }
}
