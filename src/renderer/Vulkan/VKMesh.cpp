#include "VKMesh.h"
#include "Backend/VulkanBuffer.h"
#include "../../Utils.h"
#include "Backend/VulkanCommandBlock.h"

void VKMesh::Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                  const BackendContextVariant& context){

    m_VBO = CreateMeshVBO(vertices, indices, context);
}

VulkanDeviceBuffer VKMesh::CreateMeshVBO(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                                   const BackendContextVariant& context) {
    if (auto vkContext = std::get_if<VulkanContextInfo>(&context)) {
        BufferCreateInfo bufferCreateInfo{
                .allocator = vkContext->allocator,
                .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
                .queueFamily = vkContext->queueFamily,
                .deferredDestroyBuffer = vkContext->deferredDestroyBuffer
        };
        auto verticesBytes_v = ToByteSpan(vertices);
        auto indicesBytes_v = ToByteSpan(indices);
        auto totalBytes_v =
            std::array<std::span<std::byte const>, 2>{
            verticesBytes_v,
            indicesBytes_v,
        };
        VulkanDeviceBuffer deviceBuffer(bufferCreateInfo,
            VulkanCommandBlock(vkContext->device, vkContext->queue, vkContext->commandBlockPool),
            totalBytes_v);
        return std::move(deviceBuffer);
        
    } else {
        throw std::runtime_error("Wrong context type passed to VKMesh::Init");
    }
}

void VKMesh::Draw(Material &material, vk::CommandBuffer commandBuffer) {
    commandBuffer.bindVertexBuffers(0, m_VBO.value().Get().Get(), vk::DeviceSize{});

    //todo update this to keep track of vertices and indices sizes
    commandBuffer.bindIndexBuffer(m_VBO.value().Get().Get(), 4 * sizeof(Vertex),
        vk::IndexType::eUint32);
    commandBuffer.drawIndexed(6, 1, 0, 0, 0);
}
void VKMesh::Draw(Material &material) const {
    throw std::runtime_error("Bind without command buffer not implemented vulkan mesh type");
}

VKMesh::~VKMesh() {};

