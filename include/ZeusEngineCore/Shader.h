#pragma once
#include <string>
#include <unordered_map>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

class Shader {
public:
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetUniformInt(const std::string& name, int value);
    void SetUniformMat4(const std::string& name, const glm::mat4& matrix);
    void SetUniformFloat(const std::string& name, float value);
    void SetUniformVec4(const std::string& name, const glm::vec4& value);

private:
    uint32_t m_RendererID;
    std::unordered_map<std::string, int> m_UniformLocationCache;

    int GetUniformLocation(const std::string& name);
};
