#include "VKMesh.h"
#include "Backend/VulkanBuffer.h"

void VKMesh::Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                  const BackendContextVariant& context){

    m_VBO = CreateMeshVBO(vertices, indices, context);
}

VKMesh::~VKMesh() {

}


VulkanBuffer VKMesh::CreateMeshVBO(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                                   const BackendContextVariant& context) {
    if (auto vkContext = std::get_if<VulkanContextInfo>(&context)) {
        BufferCreateInfo buffer_ci{
                .allocator = vkContext->allocator,
                .usage = vk::BufferUsageFlagBits::eVertexBuffer,
                .queue_family = vkContext->queueFamily,
        };

        VulkanBuffer buffer(buffer_ci, BufferMemoryType::Host, sizeof(Vertex) * vertices.size());

        std::memcpy(buffer.Mapped(), vertices.data(), sizeof(Vertex) * vertices.size());
        return buffer;
    } else {
        throw std::runtime_error("Wrong context type passed to VKMesh::Init");
    }
}

void VKMesh::Draw(Material &material, vk::CommandBuffer commandBuffer) {
    commandBuffer.bindVertexBuffers(0, m_VBO.value().Get(),vk::DeviceSize{});
    commandBuffer.draw(3, 1, 0, 0);
}
void VKMesh::Draw(Material &material) const {
    throw std::runtime_error("Bind without command buffer not implemented vulkan mesh type");
}

