
#pragma once
#include "ZeusEngineCore/IRenderer.h"

class VulkanRenderer : public IRenderer {
public:
    void Init() override;

    void Cleanup() override;

    void BeginFrame() override;

    void Submit() override;

    void EndFrame() override;

    void DrawMesh(glm::vec4 color) override;
};
