#include "Mesh.h"
#include "Backend/Buffer.h"
#include "../../Utils.h"
#include "Backend/CommandBlock.h"
#include "Backend/APIRenderer.h"

using namespace ZEN::VKAPI;

Mesh::Mesh(APIRenderer *apiRenderer) : m_APIRenderer(apiRenderer){

};


void Mesh::Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                const BackendContextVariant& context){

    m_VBO = CreateMeshVBO(vertices, indices, context);
}

DeviceBuffer Mesh::CreateMeshVBO(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                                 const BackendContextVariant& context) {
    if (auto vkContext = std::get_if<ContextInfo>(&context)) {
        BufferCreateInfo bufferCreateInfo{
                .allocator = vkContext->allocator,
                .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
                .queueFamily = vkContext->queueFamily,
                .destroyCallback = vkContext->destroyCallback
        };
        auto verticesBytes_v = ToByteSpan(vertices);
        auto indicesBytes_v = ToByteSpan(indices);
        auto totalBytes_v =
            std::array<std::span<std::byte const>, 2>{
            verticesBytes_v,
            indicesBytes_v,
        };
        DeviceBuffer deviceBuffer(bufferCreateInfo,
                                  CommandBlock(vkContext->device, vkContext->queue, vkContext->commandBlockPool),
                                  totalBytes_v);
        return std::move(deviceBuffer);
        
    } else {
        throw std::runtime_error("Wrong context type passed to Mesh::Init");
    }
}

void Mesh::Draw(Material &material, vk::CommandBuffer commandBuffer) {
    //todo bind material
    commandBuffer.bindVertexBuffers(0, m_VBO.value().Get().Get(), vk::DeviceSize{});

    //todo update this to keep track of vertices and indices sizes
    commandBuffer.bindIndexBuffer(m_VBO.value().Get().Get(), 4 * sizeof(Vertex),
        vk::IndexType::eUint32);
    commandBuffer.drawIndexed(6, 1, 0, 0, 0);
}
void Mesh::Draw(Material &material) const {
    //throw std::runtime_error("Bind without command buffer not implemented vulkan mesh type");
    m_APIRenderer->DrawIndexed(m_VBO.value().Get().Get());
}

Mesh::~Mesh() {}


