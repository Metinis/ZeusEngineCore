
#pragma once
#include "ZeusEngineCore/IRenderer.h"

class VulkanRenderer : public IRenderer {
public:
    void Init() override;

    void Cleanup() override;

    void BeginFrame() override;

    void Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) override;

    void EndFrame() override;

    void DrawMesh(const IMesh& mesh, Material& material) override;
};
