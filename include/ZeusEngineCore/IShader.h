#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <glm/mat4x4.hpp>

#include "RendererAPI.h"

class IShader {
public:
    virtual void Init(const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
    virtual ~IShader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetUniformInt(const std::string& name, int value) = 0;
    virtual void SetUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;
    virtual void SetUniformFloat(const std::string& name, float value) = 0;
    virtual void SetUniformVec4(const std::string& name, const glm::vec4& value) = 0;

    static std::shared_ptr<IShader> Create(RendererAPI api);

protected:
    uint32_t m_RendererID;
    std::unordered_map<std::string, int> m_UniformLocationCache;

    virtual int GetUniformLocation(const std::string& name) = 0;
};
