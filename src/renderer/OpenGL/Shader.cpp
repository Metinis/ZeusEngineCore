#include "Shader.h"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "ZeusEngineCore/Utils.h"

using namespace ZEN::OGLAPI;

Shader::Shader(const ShaderInfo& shaderInfo) {
    const std::string vertexSrc = ZEN::ReadFile(shaderInfo.vertexPath);
    const std::string fragmentSrc = ZEN::ReadFile(shaderInfo.fragmentPath);

    uint32_t vertex = glCreateShader(GL_VERTEX_SHADER);
    const char* vSrc = vertexSrc.c_str();
    glShaderSource(vertex, 1, &vSrc, nullptr);
    glCompileShader(vertex);

    // Check vertex shader compile status
    {
        GLint success = 0;
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            GLint logLength = 0;
            glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &logLength);
            std::string log(logLength, ' ');
            glGetShaderInfoLog(vertex, logLength, nullptr, &log[0]);
            std::cerr << "Vertex shader compilation failed:\n" << log << std::endl;
        }
    }

    uint32_t fragment = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSrc = fragmentSrc.c_str();
    glShaderSource(fragment, 1, &fSrc, nullptr);
    glCompileShader(fragment);

    // Check fragment shader compile status
    {
        GLint success = 0;
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            GLint logLength = 0;
            glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &logLength);
            std::string log(logLength, ' ');
            glGetShaderInfoLog(fragment, logLength, nullptr, &log[0]);
            std::cerr << "Fragment shader compilation failed:\n" << log << std::endl;
        }
    }

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vertex);
    glAttachShader(m_RendererID, fragment);
    glLinkProgram(m_RendererID);

    // Check program link status
    {
        GLint success = 0;
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
        if (success == GL_FALSE) {
            GLint logLength = 0;
            glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &logLength);
            std::string log(logLength, ' ');
            glGetProgramInfoLog(m_RendererID, logLength, nullptr, &log[0]);
            std::cerr << "Shader program linking failed:\n" << log << std::endl;
        }
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLuint viewBlockIndex = glGetUniformBlockIndex(m_RendererID, "View");
    glUniformBlockBinding(m_RendererID, viewBlockIndex, 0);

    GLuint instancesBlockIndex = glGetUniformBlockIndex(m_RendererID, "Instances");
    glUniformBlockBinding(m_RendererID, instancesBlockIndex, 1);
}


Shader::~Shader() {
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const {
    if(m_IsWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(m_LineWidth);
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


