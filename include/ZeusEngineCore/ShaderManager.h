#pragma once
#include <memory>

#include "../../src/renderer/OpenGL/GLShader.h"

class ShaderManager {
public:
    ShaderManager(const ShaderInfo& shaderInfo);
    std::shared_ptr<IShader> Load(const std::string &name, const std::string& vertexPath,
    const std::string& fragmentPath);
    std::shared_ptr<IShader> Get(const std::string& name);

private:
    std::unordered_map<std::string, std::shared_ptr<IShader>> m_Shaders;

    ShaderInfo m_ShaderInfo;
};
