#include "GLShader.h"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "../../Utils.h"


void GLShader::Init(const ShaderInfo& shaderInfo) {
    const std::string vertexSrc = ReadFile(shaderInfo.vertexPath);
    const std::string fragmentSrc = ReadFile(shaderInfo.fragmentPath);

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
GLShader::~GLShader() {
    glDeleteProgram(m_RendererID);
}

void GLShader::Bind() const {
    glUseProgram(m_RendererID);
}

void GLShader::Unbind() const {
    glUseProgram(0);
}

void GLShader::SetUniformInt(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}
void GLShader::SetUniformVec4(const std::string& name, const glm::vec4& value) {
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}
void GLShader::SetUniformFloat(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}
void GLShader::SetUniformMat4(const std::string& name, const glm::mat4& matrix) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

int GLShader::GetUniformLocation(const std::string& name) {
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    int location = glGetUniformLocation(m_RendererID, name.c_str());
    m_UniformLocationCache[name] = location;
    return location;
}
