#pragma once
#include "ZeusEngineCore/IRenderer.h"
#include "ZeusEngineCore/Shader.h"

class OpenGLRenderer : public IRenderer{
public:
    void Init() override;

    void Cleanup() override;

    void BeginFrame() override;

    void Submit() override;

    void EndFrame() override;

    void DrawMesh(glm::vec4 color) override;


private:
    std::unique_ptr<Shader> m_Shader;
    unsigned int m_VAO = 0, m_VBO = 0;
};
