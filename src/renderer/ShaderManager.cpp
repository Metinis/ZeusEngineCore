#include "ZeusEngineCore/ShaderManager.h"
#include <unordered_map>

using namespace ZEN;

ShaderManager::ShaderManager(const ShaderInfo& shaderInfo) : m_ShaderInfo(shaderInfo)
{   
   
}
std::shared_ptr<IShader> ShaderManager::Load(const std::string &name, const std::string& vertexPath,
    const std::string& fragmentPath) {
    auto it = m_Shaders.find(name);
    if(it != m_Shaders.end())
        return it->second;

    auto shader = IShader::Create(m_ShaderInfo.api);
    m_ShaderInfo.vertexPath = vertexPath;
    m_ShaderInfo.fragmentPath = fragmentPath;
    shader->Init(m_ShaderInfo);
    m_Shaders[name] = shader;
    return shader;
}
std::shared_ptr<IShader> ShaderManager::Get(const std::string &name) {
    auto it = m_Shaders.find(name);
    if(it != m_Shaders.end())
        return it->second;
    return nullptr;
}
