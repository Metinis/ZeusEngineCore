#pragma once
#include <memory>

#include "../../src/renderer/OpenGL/GLShader.h"

class ShaderManager {
public:
    static std::shared_ptr<IShader> Load(const std::string &name, const std::string &vertexSrc,
    const std::string &fragmentSrc, RendererAPI api);
    static std::shared_ptr<IShader> Get(const std::string& name);

private:
    static std::unordered_map<std::string, std::shared_ptr<IShader>> s_Shaders;
};
