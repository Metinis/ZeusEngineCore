#pragma once
#include "ZeusEngineCore/IRenderer.h"

class OpenGLRenderer : public IRenderer{
public:
    void Init() override;

    void Cleanup() override;

    void BeginFrame() override;

    void Submit() override;

    void EndFrame() override;

    void DrawMesh(const IMesh& mesh, Material& material) override;

};
