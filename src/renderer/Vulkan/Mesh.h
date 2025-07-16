#pragma once
#include "ZeusEngineCore/IMesh.h"
#include "Backend/DeviceBuffer.h"
#include <optional>

namespace ZEN::VKAPI {
    class APIRenderer;
    struct MeshInfo{
        VmaAllocator allocator;
        vk::Device device;
        vk::Queue queue;
        std::uint32_t queueFamily;
        vk::CommandPool commandBlockPool;
        std::shared_ptr<std::function<void(DeferredHandle)>> destroyCallback;
        APIRenderer* apiRenderer;
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
    };
    class Mesh : public IMesh {
    public:
        explicit Mesh(const MeshInfo& meshInfo);

        ~Mesh() override;

        void Draw() const override;

    private:
        DeviceBuffer CreateMeshVBO(const MeshInfo& meshInfo);

        std::optional<DeviceBuffer> m_VBO;
        APIRenderer* m_APIRenderer;
    };
}
