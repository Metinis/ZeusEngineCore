#pragma once
#include "ZeusEngineCore/IMesh.h"
#include "Backend/Buffer.h"
#include "Backend/DeviceBuffer.h"
#include <optional>

namespace ZEN::VKAPI {
    class Mesh : public IMesh {
    public:
        void Init(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
                  const BackendContextVariant &context) override;

        ~Mesh() override;

        void Draw(Material &material) const override;

        void Draw(Material &material, vk::CommandBuffer commandBuffer) override;

    private:
        DeviceBuffer CreateMeshVBO(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
                                   const BackendContextVariant &context);

        std::optional<DeviceBuffer> m_VBO;
    };
}
