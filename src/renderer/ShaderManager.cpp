#include "ZeusEngineCore/ShaderManager.h"
#include <unordered_map>
std::unordered_map<std::string, std::shared_ptr<IShader>> ShaderManager::s_Shaders;

std::shared_ptr<IShader> ShaderManager::Load(const std::string &name, const std::string &vertexSrc,
    const std::string &fragmentSrc, const RendererAPI api) {
    auto it = s_Shaders.find(name);
    if(it != s_Shaders.end())
        return it->second;

    //auto shader = std::make_shared<GLShader>();
    auto shader = IShader::Create(api);
    shader->Init(vertexSrc, fragmentSrc);
    s_Shaders[name] = shader;
    return shader;
}
std::shared_ptr<IShader> ShaderManager::Get(const std::string &name) {
    auto it = s_Shaders.find(name);
    if(it != s_Shaders.end())
        return it->second;
    return nullptr;
}

