#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>

#include "VKInit.h"
#include "ZeusEngineCore/engine/rendering/VKUtils.h"

using namespace ZEN;

GPUMeshBuffers VKRenderer::uploadMesh(AssetID id, const MeshData &mesh) {

    if (m_MeshMap.find(id) != m_MeshMap.end()) {
        spdlog::warn("AssetID already exists in GPU, overwriting..");
        deleteMesh(id);
    }

    const size_t vertexBufferSize = mesh.vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = mesh.indices.size() * sizeof(uint32_t);

    GPUMeshBuffers newSurface;
    newSurface.vertexBuffer = createBuffer(vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAddressInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = newSurface.vertexBuffer.buffer,
    };
    newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_Device, &deviceAddressInfo);

    newSurface.indexBuffer = createBuffer(indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = createBuffer(vertexBufferSize + indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = staging.allocationInfo.pMappedData;

    memcpy(data, mesh.vertices.data(), vertexBufferSize);
    memcpy((char*)data + vertexBufferSize, mesh.indices.data(), indexBufferSize);

    immediateSubmit([&](VkCommandBuffer cmd) {
        VkBufferCopy vertCopy{0};
        vertCopy.dstOffset = 0;
        vertCopy.srcOffset = 0;
        vertCopy.size = vertexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer,
            newSurface.vertexBuffer.buffer, 1, &vertCopy);

        VkBufferCopy indexCopy{0};
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer,
            newSurface.indexBuffer.buffer, 1, &indexCopy);
    });
    newSurface.indexCount = mesh.indices.size();

    destroyBuffer(staging);
    m_MeshMap[id] = newSurface;

    spdlog::debug("Renderer: Created GPU Mesh ID: {} of index count: {}", (uint64_t)id, newSurface.indexCount);
    return newSurface;
}

GPUTexture VKRenderer::uploadTexture(AssetID id, const TextureData &texture) {
    int texWidth, texHeight, texChannels;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels;
    bool allocatedByUs = true;

    //---------------------ASSIMP TEXTURE--------------------
    if (texture.aiTex && texture.aiTex->mHeight == 0) {
        pixels = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture.aiTex->pcData),texture.aiTex->mWidth,
    &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    } else if (texture.aiTex) {
        texWidth = texture.aiTex->mWidth;
        texHeight = texture.aiTex->mHeight;
        texChannels = 4;
        pixels = reinterpret_cast<unsigned char*>(texture.aiTex->pcData);
        allocatedByUs = false;
    //--------------------------------------------------------
    //---------------------PATH TEXTURE-----------------------
    } else {
        pixels = stbi_load(texture.path.data(), &texWidth,
                                    &texHeight, &texChannels, STBI_rgb_alpha);
    }
    //--------------------------------------------------------

    if (!pixels && allocatedByUs) {
        std::cout<<"Invalid Image! Assigning default texture.."<<"\n";
        return {};
    }

    AllocatedImage newTexture = createImage((void*)pixels, VkExtent3D{(unsigned int)texWidth, (unsigned int)texHeight, 1}, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT);

    uint32_t index = m_TextureAllocator.allocate();

    GPUTexture ret = {
        .image = newTexture,
        .sampler = m_DefaultSamplerNearest,
        .index = index,
    };
    DescriptorWriter writer;

    writer.writeImage(0, ret.image.imageView, ret.sampler,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ret.index);
    writer.updateSet(m_Device, m_TextureDescriptorSet);

    stbi_image_free(pixels);

    m_TextureMap[id] = ret;

    spdlog::debug("Renderer: Created Texture ID: {}", (uint64_t)id);

    return ret;
}

GPUMaterial VKRenderer::uploadMaterial(AssetID id, const Material &material) {
    if (m_MaterialMap.find(id) != m_MaterialMap.end()) {
        spdlog::warn("Attempt to upload material ID that exists: {}", (uint64_t)id);
    }

    GPUMaterial gpuMat = {
        .u_Albedo = glm::vec4(material.albedo.x, material.albedo.y, material.albedo.z, 1.0f),
        .u_Params = glm::vec4(material.metallic, material.roughness, material.ao, 1.0),
    };

    if (m_TextureMap.contains(material.texture)) {
        gpuMat.albedoIndex = m_TextureMap[material.texture].index;
    }
    if (m_TextureMap.contains(material.metallicTex)) {
        gpuMat.metallicIndex = m_TextureMap[material.metallicTex].index;
    }
    if (m_TextureMap.contains(material.roughnessTex)) {
        gpuMat.roughnessIndex = m_TextureMap[material.roughnessTex].index;
    }
    if (m_TextureMap.contains(material.normalTex)) {
        gpuMat.normalIndex = m_TextureMap[material.normalTex].index;
    }
    if (m_TextureMap.contains(material.aoTex)) {
        gpuMat.aoIndex = m_TextureMap[material.aoTex].index;
    }

    DescriptorWriter writer;

    uint32_t idx = m_MaterialAllocator.allocate();

    gpuMat.idx = idx;
    m_MaterialMap[id] = gpuMat;

    auto* mapped = (GPUMaterial*)m_MaterialBuffer.allocationInfo.pMappedData;
    mapped[idx] = gpuMat;
}

void VKRenderer::deleteMaterial(AssetID id) {
    if (m_MaterialMap.find(id) != m_MaterialMap.end()) {
        auto& material = m_MaterialMap[id];//todo free associated index
        //m_MaterialAllocator.free();
        m_MaterialMap.erase(id);
    }
}

void VKRenderer::deleteMesh(AssetID id) {
    if (m_MeshMap.find(id) != m_MeshMap.end()) {
        auto meshBuf = m_MeshMap[id];
        getCurrentFrame().m_DeletionQueue.pushFunction([=]() {
            destroyBuffer(meshBuf.indexBuffer);
            destroyBuffer(meshBuf.vertexBuffer);
        });
        m_MeshMap.erase(id);
        spdlog::debug("Deleted GPU Mesh ID: {}", (uint64_t)id);
        return;
    }
    spdlog::error("Attempt to delete non-existing GPU Mesh! ID: {}", (uint64_t)id);
}

void VKRenderer::removeTexture(AssetID id) {
    if (m_TextureMap.find(id) != m_TextureMap.end()) {
        auto tex = m_TextureMap[id];
        uint32_t frameIndex = (m_FrameNumber + FRAME_OVERLAP) % FRAME_OVERLAP;
        m_Frames[frameIndex].m_DeletionQueue.pushFunction([=]() {
            if (m_ImGUIDescSetMap.contains(id)) {
                ImGui_ImplVulkan_RemoveTexture(m_ImGUIDescSetMap[id]);
                m_ImGUIDescSetMap.erase(id);
            }
            m_TextureMap.erase(id);
            m_TextureAllocator.free(tex.index);
            destroyImage(tex.image);
        });
        spdlog::debug("Deleted Texture ID: {}", (uint64_t)id);
        return;
    }
    spdlog::error("Attempt to delete non-existing Texture! ID: {}", (uint64_t)id);
}

AllocatedImage VKRenderer::createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
    AllocatedImage newImage;
    newImage.imageFormat = format;
    newImage.imageExtent = size;

    VkImageCreateInfo imgInfo = VKInit::imageCreateInfo(format, usage, size);
    if (mipmapped) {
        imgInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
    }

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vmaCreateImage(m_Allocator, &imgInfo, &allocInfo, &newImage.image, &newImage.allocation, nullptr));

    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format == VK_FORMAT_D32_SFLOAT) {
        aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    VkImageViewCreateInfo viewInfo = VKInit::imageViewCreateInfo(newImage.image, format, aspectFlag);
    viewInfo.subresourceRange.levelCount = imgInfo.mipLevels;

    VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &newImage.imageView));

    spdlog::debug("Renderer: Created Image");

    return newImage;
}

AllocatedImage VKRenderer::createImage(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
    bool mipmapped) {
    size_t dataSize = size.depth * size.width * size.height * 4; //todo check if can use sizeof here
    AllocatedBuffer uploadBuffer = createBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    memcpy(uploadBuffer.allocationInfo.pMappedData, data, dataSize);

    AllocatedImage newImage = createImage(size, format,
        usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

    immediateSubmit([&](VkCommandBuffer cmd) {
        VKImages::transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = size;

        vkCmdCopyBufferToImage(cmd, uploadBuffer.buffer, newImage.image,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
            &copyRegion);

        VKImages::transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        spdlog::debug("Renderer: Uploaded Image");
    });

    destroyBuffer(uploadBuffer);
    return newImage;
}

void VKRenderer::destroyImage(const AllocatedImage &img) {
    vkDestroyImageView(m_Device, img.imageView, nullptr);
    vmaDestroyImage(m_Allocator, img.image, img.allocation);
    spdlog::debug("Renderer: Deleted Image");
}

AllocatedBuffer VKRenderer::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;
    bufferInfo.pNext = nullptr;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    AllocatedBuffer buffer;

    VK_CHECK(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &buffer.buffer,
        &buffer.allocation, &buffer.allocationInfo));

    //spdlog::debug("Renderer: Buffer Created");

    return buffer;
}

void VKRenderer::destroyBuffer(const AllocatedBuffer &buffer) {
    vmaDestroyBuffer(m_Allocator, buffer.buffer, buffer.allocation);
    //spdlog::debug("Renderer: Buffer Destroyed");
}
