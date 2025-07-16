#include "Mesh.h"
#include "Backend/Buffer.h"
#include "../../Utils.h"
#include "Backend/CommandBlock.h"
#include "Backend/APIRenderer.h"

using namespace ZEN::VKAPI;

Mesh::Mesh(const MeshInfo& meshInfo){
    m_VBO = CreateMeshVBO(meshInfo);
    m_APIRenderer = meshInfo.apiRenderer;
};

DeviceBuffer Mesh::CreateMeshVBO(const MeshInfo& meshInfo) {
        BufferCreateInfo bufferCreateInfo{
                .allocator = meshInfo.allocator,
                .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
                .queueFamily = meshInfo.queueFamily,
                .destroyCallback = meshInfo.destroyCallback
        };
        auto verticesBytes_v = ToByteSpan(meshInfo.vertices);
        auto indicesBytes_v = ToByteSpan(meshInfo.indices);
        auto totalBytes_v =
            std::array<std::span<std::byte const>, 2>{
            verticesBytes_v,
            indicesBytes_v,
        };
        DeviceBuffer deviceBuffer(bufferCreateInfo,
                                  CommandBlock(meshInfo.device,
                                               meshInfo.queue,
                                               meshInfo.commandBlockPool),
                                  totalBytes_v);
        return std::move(deviceBuffer);
}
void Mesh::Draw() const {
    m_APIRenderer->DrawIndexed(m_VBO.value().Get().Get());
}

Mesh::~Mesh() {}


