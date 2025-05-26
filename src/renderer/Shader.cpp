#include "ZeusEngineCore/Shader.h"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>


Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
    uint32_t vertex = glCreateShader(GL_VERTEX_SHADER);
    const char* vSrc = vertexSrc.c_str();
    glShaderSource(vertex, 1, &vSrc, nullptr);
    glCompileShader(vertex);

    uint32_t fragment = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSrc = fragmentSrc.c_str();
    glShaderSource(fragment, 1, &fSrc, nullptr);
    glCompileShader(fragment);

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vertex);
    glAttachShader(m_RendererID, fragment);
    glLinkProgram(m_RendererID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const {
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

void Shader::SetUniformInt(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}
void Shader::SetUniformVec4(const std::string& name, const glm::vec4& value) {
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}
void Shader::SetUniformFloat(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}
void Shader::SetUniformMat4(const std::string& name, const glm::mat4& matrix) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

int Shader::GetUniformLocation(const std::string& name) {
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    int location = glGetUniformLocation(m_RendererID, name.c_str());
    m_UniformLocationCache[name] = location;
    return location;
}
