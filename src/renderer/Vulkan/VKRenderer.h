
#pragma once
#include "ZeusEngineCore/IRenderer.h"
#include "Backend/VulkanBackend.h"

class VKRenderer : public IRenderer {
public:
    void Init(RendererInitInfo& initInfo) override;

    ~VKRenderer() override;

    bool BeginFrame() override;

    void Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) override;

    void EndFrame() override;

    void DrawMesh(const IMesh& mesh, Material& material) override;
private:
    std::unique_ptr<VulkanBackend> m_VKBackend;
    vk::CommandBuffer m_CommandBuffer;
};
