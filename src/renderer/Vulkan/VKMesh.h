#pragma once
#include "ZeusEngineCore/IMesh.h"
#include "Backend/VulkanBuffer.h"
#include "Backend/VulkanDeviceBuffer.h"
#include <optional>

class VKMesh : public IMesh{
public:
    void Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
              const BackendContextVariant& context) override;
    ~VKMesh() override;
    void Draw(Material& material) const override;
    void Draw(Material& material, vk::CommandBuffer commandBuffer) override;
private:
    VulkanDeviceBuffer CreateMeshVBO(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                               const BackendContextVariant& context);
    std::optional<VulkanDeviceBuffer> m_VBO;
};
