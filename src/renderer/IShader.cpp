#include "ZeusEngineCore/IShader.h"
#include "OpenGL/Shader.h"
#include "Vulkan/ShaderPipeline.h"
#include "ZeusEngineCore/IRendererAPI.h"
#include "Vulkan/Backend/APIBackend.h"
#include "Vulkan/Backend/APIRenderer.h"
#include "OpenGL/APIBackend.h"
#include "OpenGL/APIRenderer.h"

using namespace ZEN;

std::shared_ptr<IShader> IShader::Create(IRendererBackend* apiBackend,
                                         IRendererAPI* apiRenderer,
                                         const std::string& vertexPath,
                                         const std::string& fragmentPath) {
    switch(apiBackend->GetAPI()) {
        case eRendererAPI::OpenGL:
        {
            auto* backendAPI = dynamic_cast<OGLAPI::APIBackend*>(apiBackend);
            auto* rendererAPI = dynamic_cast<OGLAPI::APIRenderer*>(apiRenderer);
            OGLAPI::ShaderInfo info = backendAPI->GetShaderInfo();
            info.apiRenderer = rendererAPI;
            info.vertexPath = vertexPath;
            info.fragmentPath = fragmentPath;
            return std::make_shared<OGLAPI::Shader>(info);
        }
        case eRendererAPI::Vulkan:
        {
            auto* backendAPI = dynamic_cast<VKAPI::APIBackend*>(apiBackend);
            auto* rendererAPI = dynamic_cast<VKAPI::APIRenderer*>(apiRenderer);
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
