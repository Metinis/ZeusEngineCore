#pragma once
#include <memory>

#include "Shader.h"

class ShaderManager {
public:
    static std::shared_ptr<Shader> Load(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
    static std::shared_ptr<Shader> Get(const std::string& name);

private:
    static std::unordered_map<std::string, std::shared_ptr<Shader>> s_Shaders;
};
