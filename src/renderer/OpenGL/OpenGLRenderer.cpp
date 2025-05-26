#include "OpenGLRenderer.h"

#include <memory>
#include <string>
#include <glad/glad.h>
#include <glm/vec4.hpp>

#include "ZeusEngineCore/Shader.h"

void OpenGLRenderer::Init() {
    const std::string vertexSrc = R"(
        #version 410 core
        layout(location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const std::string fragmentSrc = R"(
        #version 410 core
        out vec4 FragColor;
        uniform vec4 u_Color;
        void main() {
            FragColor = u_Color;
        }
    )";

    m_Shader = std::make_unique<Shader>(vertexSrc, fragmentSrc);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
}

void OpenGLRenderer::Cleanup() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

void OpenGLRenderer::BeginFrame() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::Submit() {

}


void OpenGLRenderer::EndFrame() {
//glfw swapping handled in Window
    //draw submitted commands

}

void OpenGLRenderer::DrawMesh(glm::vec4 color) {
    //immediately draw (costly for performance but option available)
    m_Shader->Bind();
    m_Shader->SetUniformVec4("u_Color", color);
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
