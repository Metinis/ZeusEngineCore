#include "ZeusEngineCore/IShader.h"
#include "OpenGL/Shader.h"
#include "Vulkan/ShaderPipeline.h"
#include "ZeusEngineCore/RendererAPI.h"
#include "Vulkan/Backend/APIBackend.h"

using namespace ZEN;

std::shared_ptr<IShader> IShader::Create(VKAPI::APIBackend* apiBackend,
                                         VKAPI::APIRenderer* apiRenderer,
                                         const std::string& vertexPath,
                                         const std::string& fragmentPath) {
    switch(apiBackend->GetAPI()) {
        case RendererAPI::OpenGL: return std::make_shared<OGLAPI::Shader>();
        case RendererAPI::Vulkan:
        {
            VKAPI::APIBackend* backendAPI = apiBackend;//static_cast<VKAPI::APIBackend>(apiBackend);
            VKAPI::APIRenderer* rendererAPI = apiRenderer;
            VKAPI::ShaderInfo info = backendAPI->GetShaderInfo();
            info.apiRenderer = rendererAPI;
            info.vertexPath = vertexPath;
            info.fragmentPath = fragmentPath;
            return std::make_shared<VKAPI::ShaderPipeline>(info);
        }

        default: return nullptr;
    }
    //return nullptr;
}
