
#pragma once
#include "ZeusEngineCore/IMesh.h"
#include "Backend/VulkanBuffer.h"

class VKMesh : public IMesh{
public:
    void Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
              const BackendContextVariant& context) override;
    ~VKMesh() override;
    void Draw(Material& material) const override;
    void Draw(Material& material, vk::CommandBuffer commandBuffer) override;
private:
    VulkanBuffer CreateMeshVBO(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                               const BackendContextVariant& context);
    std::optional<VulkanBuffer> m_VBO;
};
