#include "ZeusEngineCore/ShaderManager.h"
#include "ZeusEngineCore/IShader.h"
#include <string>


using namespace ZEN;

ShaderManager::ShaderManager(IRendererBackend* backendAPI, IRendererAPI* rendererAPI)
: m_BackendAPI(backendAPI),
m_RendererAPI(rendererAPI)
{   
   
}
std::shared_ptr<IShader> ShaderManager::Load(const std::string &name, const std::string& vertexPath,
    const std::string& fragmentPath) {
    auto it = m_Shaders.find(name);
    if(it != m_Shaders.end())
        return it->second;

    auto shader = IShader::Create(m_BackendAPI, m_RendererAPI, vertexPath, fragmentPath);
    m_Shaders[name] = shader;
    return shader;
}
std::shared_ptr<IShader> ShaderManager::Get(const std::string &name) {
    auto it = m_Shaders.find(name);
    if(it != m_Shaders.end())
        return it->second;
    return nullptr;
}
