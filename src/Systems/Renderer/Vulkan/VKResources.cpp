#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>

#include "VkHelpers.h"
#include "VKInit.h"
#include "ZeusEngineCore/core/Project.h"
#include "ZeusEngineCore/engine/rendering/VKUtils.h"

using namespace ZEN;

GPUMeshBuffers VKRenderer::uploadMesh(AssetID id, const MeshData &mesh) {
    if (m_ResourceCtx.m_MeshMap.contains(id)) {
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

    VkBufferDeviceAddressInfo deviceAddressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = newSurface.vertexBuffer.buffer,
    };
    newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_StateCtx.m_Device, &deviceAddressInfo);

    newSurface.indexBuffer = createBuffer(indexBufferSize,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                                          | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = createBuffer(vertexBufferSize + indexBufferSize,
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void *data = staging.allocationInfo.pMappedData;

    memcpy(data, mesh.vertices.data(), vertexBufferSize);
    memcpy((char *) data + vertexBufferSize, mesh.indices.data(), indexBufferSize);

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
    m_ResourceCtx.m_MeshMap[id] = newSurface;

    spdlog::debug("Renderer: Created GPU Mesh ID: {} of index count: {}", (uint64_t) id, newSurface.indexCount);
    return newSurface;
}

struct LoadedTexture {
    std::vector<uint8_t> pixels{};
    int texWidth{};
    int texHeight{};
    int texChannels{};
    int layers{1};
};

static LoadedTexture loadPixelData(const TextureData &texture) {
    stbi_set_flip_vertically_on_load(true);
    constexpr size_t bytesPerPixel = 4;

    LoadedTexture loadedTexture{};

    auto calcSize = [&](int width, int height) {
        return static_cast<size_t>(width) *
               static_cast<size_t>(height) *
               bytesPerPixel;
    };

    switch (texture.type) {
        case Texture2DAssimp: {
            stbi_uc *pixels = nullptr;
            bool freePixels = true;

            if (texture.aiTex && texture.aiTex->mHeight == 0) {
                pixels = stbi_load_from_memory(
                    reinterpret_cast<unsigned char *>(texture.aiTex->pcData),
                    texture.aiTex->mWidth,
                    &loadedTexture.texWidth,
                    &loadedTexture.texHeight,
                    &loadedTexture.texChannels,
                    STBI_rgb_alpha
                );
            } else if (texture.aiTex) {
                loadedTexture.texWidth = texture.aiTex->mWidth;
                loadedTexture.texHeight = texture.aiTex->mHeight;
                loadedTexture.texChannels = 4;

                pixels = reinterpret_cast<unsigned char *>(texture.aiTex->pcData);
                freePixels = false;
            } else if (!texture.path.empty()) {
                //todo will move to runtime asset manager anyways
                pixels = stbi_load(std::string(Project::getActive()->getActiveProjectRoot() + texture.path).c_str(),
                                   &loadedTexture.texWidth,
                                   &loadedTexture.texHeight, &loadedTexture.texChannels, STBI_rgb_alpha);
            }

            if (!pixels) {
                throw std::runtime_error("Failed to load Assimp texture");
            }

            size_t size = calcSize(loadedTexture.texWidth, loadedTexture.texHeight);

            loadedTexture.pixels.resize(size);
            memcpy(loadedTexture.pixels.data(), pixels, size);

            if (freePixels) {
                stbi_image_free(pixels);
            }

            return loadedTexture;
        }

        case Texture2D: {
            stbi_uc *pixels = stbi_load(
                texture.path.c_str(),
                &loadedTexture.texWidth,
                &loadedTexture.texHeight,
                &loadedTexture.texChannels,
                STBI_rgb_alpha
            );

            if (!pixels) {
                throw std::runtime_error("Failed to load texture: " + texture.path);
            }

            size_t size = calcSize(loadedTexture.texWidth, loadedTexture.texHeight);

            loadedTexture.pixels.resize(size);
            memcpy(loadedTexture.pixels.data(), pixels, size);

            stbi_image_free(pixels);

            return loadedTexture;
        }
        case Texture2DHDR: {
            float *pixels = stbi_loadf(
                texture.path.c_str(),
                &loadedTexture.texWidth,
                &loadedTexture.texHeight,
                &loadedTexture.texChannels,
                STBI_rgb_alpha
            );

            if (!pixels) {
                throw std::runtime_error("Failed to load texture: " + texture.path);
            }

            const size_t size =
                    size_t(loadedTexture.texWidth) *
                    size_t(loadedTexture.texHeight) *
                    4 *
                    sizeof(float);

            loadedTexture.pixels.resize(size);
            memcpy(loadedTexture.pixels.data(), pixels, size);

            stbi_image_free(pixels);

            return loadedTexture;
        }
        case Cubemap: {
            stbi_set_flip_vertically_on_load(false);

            std::array<std::string, 6> facePaths = {
                "right.jpg",
                "left.jpg",
                "top.jpg",
                "bottom.jpg",
                "front.jpg",
                "back.jpg"
            };

            std::array<stbi_uc *, 6> faces{};

            for (size_t i{}; i < faces.size(); ++i) {
                std::string fullPath = texture.path + facePaths[i];

                faces[i] = stbi_load(
                    fullPath.c_str(),
                    &loadedTexture.texWidth,
                    &loadedTexture.texHeight,
                    &loadedTexture.texChannels,
                    STBI_rgb_alpha
                );

                if (!faces[i]) {
                    throw std::runtime_error(
                        "Failed to load cubemap face: " + fullPath
                    );
                }
            }

            size_t faceSize = calcSize(loadedTexture.texWidth, loadedTexture.texHeight);
            loadedTexture.pixels.resize(faceSize * 6);
            loadedTexture.layers = 6;

            for (size_t i{}; i < faces.size(); ++i) {
                memcpy(
                    loadedTexture.pixels.data() + (faceSize * i),
                    faces[i],
                    faceSize
                );

                stbi_image_free(faces[i]);
            }

            return loadedTexture;
        }

        case Texture2DRaw:
            throw std::runtime_error("Texture2DRaw not implemented");

        default:
            throw std::runtime_error("Unknown texture type");
    }
}

GPUTexture VKRenderer::uploadTexture(AssetID id, const TextureData &texture) {
    LoadedTexture texturePixels = loadPixelData(texture);
    AllocatedImage newTexture = createImage((void *) texturePixels.pixels.data(),
                                            VkExtent3D{
                                                (unsigned int) texturePixels.texWidth,
                                                (unsigned int) texturePixels.texHeight, 1
                                            },
                                            texture.format, VK_IMAGE_USAGE_SAMPLED_BIT, false, texturePixels.layers);

    StoredSampler sampler = getSampler(VKHelpers::toVkSamplerCreateInfo(texture.samplerInfo));
    //--------------------------------------------------------
    if (m_ResourceCtx.m_TextureMap.contains(id)) {
        destroyImage(m_ResourceCtx.m_TextureMap[id].texture.image);
        m_ResourceCtx.m_TextureAllocator.free(m_ResourceCtx.m_TextureMap[id].idx);
    }

    GPUTexture gpuTex = {
        .image = newTexture,
        .samplerIdx = sampler.idx,
    };

    m_ResourceCtx.m_TextureMap[id] = {gpuTex, newTexture.readIdx, texture.type};
    spdlog::debug("Renderer: Created Texture ID: {}", (uint64_t) id);
    return gpuTex;
}

GPUMaterial VKRenderer::uploadMaterial(const AssetID id, const Material &material) {
    GPUMaterial gpuMat = {
        .u_Albedo = glm::vec4(material.albedo.x, material.albedo.y, material.albedo.z, 1.0f),
        .u_Params = glm::vec4(material.metallic, material.roughness, material.ao, 1.0),
    };

    if (m_ResourceCtx.m_TextureMap.contains(material.texture)) {
        gpuMat.albedoIndex = m_ResourceCtx.m_TextureMap[material.texture].idx;
    }
    if (m_ResourceCtx.m_TextureMap.contains(material.metallicTex)) {
        gpuMat.metallicIndex = m_ResourceCtx.m_TextureMap[material.metallicTex].idx;
    }
    if (m_ResourceCtx.m_TextureMap.contains(material.roughnessTex)) {
        gpuMat.roughnessIndex = m_ResourceCtx.m_TextureMap[material.roughnessTex].idx;
    }
    if (m_ResourceCtx.m_TextureMap.contains(material.normalTex)) {
        gpuMat.normalIndex = m_ResourceCtx.m_TextureMap[material.normalTex].idx;
    }
    if (m_ResourceCtx.m_TextureMap.contains(material.aoTex)) {
        gpuMat.aoIndex = m_ResourceCtx.m_TextureMap[material.aoTex].idx;
    }

    //todo do this appropriately
    gpuMat.samplerIndex = m_ResourceCtx.m_TextureMap[material.texture].texture.samplerIdx;

    uint32_t flags{};
    if (material.useAlbedo) {
        flags |= USE_ALBEDO;
    }
    if (material.useMetallic) {
        flags |= USE_METALLIC;
    }
    if (material.useRoughness) {
        flags |= USE_ROUGHNESS;
    }
    if (material.useNormal) {
        flags |= USE_NORMAL;
    }
    if (material.useAO) {
        flags |= USE_AO;
    }

    gpuMat.flags = flags;

    VkPipeline pipeline;
    if (m_ResourceCtx.m_PipelineMap.contains(material.pipelineInfo)) {
        pipeline = m_ResourceCtx.m_PipelineMap[material.pipelineInfo];
    } else {
        pipeline = createMainPipeline(material.pipelineInfo);
    }
    bool useDepth = material.pipelineInfo.depthTestEnabled;

    DescriptorWriter writer;

    uint32_t idx{};

    //create new, otherwise replace
    if (!m_ResourceCtx.m_MaterialMap.contains(id)) {
        idx = m_ResourceCtx.m_MaterialAllocator.allocate();
    } else {
        idx = m_ResourceCtx.m_MaterialMap[id].idx;
    }

    m_ResourceCtx.m_MaterialMap[id] = {gpuMat, pipeline, useDepth, idx};

    auto *mapped = (GPUMaterial *) m_ResourceCtx.m_MaterialBuffer.allocationInfo.pMappedData;
    mapped[idx] = gpuMat;
    spdlog::debug("Renderer: Created Material ID: {}", (uint64_t) id);
    return gpuMat;
}

void VKRenderer::deleteMaterial(AssetID id) {
    if (m_ResourceCtx.m_MaterialMap.contains(id)) {
        auto material = m_ResourceCtx.m_MaterialMap[id];
        uint32_t frameIndex = (m_RenderCtx.m_FrameNumber + FRAME_OVERLAP) % FRAME_OVERLAP;
        m_RenderCtx.m_Frames[frameIndex].m_DeletionQueue.pushFunction([=]() {
            m_ResourceCtx.m_MaterialAllocator.free(material.idx);
        });
        m_ResourceCtx.m_MaterialMap.erase(id);

        spdlog::debug("Renderer: Deleted Material ID: {}", (uint64_t) id);
        return;
    }
    spdlog::error("Renderer: Attempt to delete non-existing Material! ID: {}", (uint64_t) id);
}

void VKRenderer::deleteMesh(AssetID id) {
    if (m_ResourceCtx.m_MeshMap.contains(id)) {
        auto &meshBuf = m_ResourceCtx.m_MeshMap[id];
        uint32_t frameIndex = (m_RenderCtx.m_FrameNumber + FRAME_OVERLAP) % FRAME_OVERLAP;
        m_RenderCtx.m_Frames[frameIndex].m_DeletionQueue.pushFunction([=]() {
            destroyBuffer(meshBuf.indexBuffer);
            destroyBuffer(meshBuf.vertexBuffer);
        });
        m_ResourceCtx.m_MeshMap.erase(id);
        spdlog::debug("Deleted GPU Mesh ID: {}", (uint64_t) id);
        return;
    }
    spdlog::error("Attempt to delete non-existing GPU Mesh! ID: {}", (uint64_t) id);
}

void VKRenderer::deleteTexture(AssetID id) {
    if (m_ResourceCtx.m_TextureMap.contains(id)) {
        auto tex = m_ResourceCtx.m_TextureMap[id];
        uint32_t frameIndex = (m_RenderCtx.m_FrameNumber + FRAME_OVERLAP) % FRAME_OVERLAP;
        m_RenderCtx.m_Frames[frameIndex].m_DeletionQueue.pushFunction([=]() {
            if (m_ResourceCtx.m_ImGUIDescSetMap.contains(id)) {
                ImGui_ImplVulkan_RemoveTexture(m_ResourceCtx.m_ImGUIDescSetMap[id]);
                m_ResourceCtx.m_ImGUIDescSetMap.erase(id);
            }
            m_ResourceCtx.m_TextureAllocator.free(tex.idx);
            destroyImage(tex.texture.image);
        });
        m_ResourceCtx.m_TextureMap.erase(id);
        spdlog::debug("Deleted Texture ID: {}", (uint64_t) id);
        return;
    }
    spdlog::error("Attempt to delete non-existing Texture! ID: {}", (uint64_t) id);
}

static size_t formatSize(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
            return 4;

        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return 8;

        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return 16;

        default:
            throw std::runtime_error("Unsupported format size");
    }
}

AllocatedImage VKRenderer::createImage(VkExtent3D size, VkFormat format,
                                       VkImageUsageFlags usage,
                                       bool mipmapped,
                                       int layers) {
    AllocatedImage img;
    img.imageFormat = format;
    img.imageExtent = size;

    bool isCubeMap = (layers == 6);

    VkImageCreateInfo imgInfo = VKInit::imageCreateInfo(format, usage, size);

    if (mipmapped)
        imgInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
    else
        imgInfo.mipLevels = 1;

    img.mipLevels = imgInfo.mipLevels;
    imgInfo.arrayLayers = layers;

    if (isCubeMap)
        imgInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VK_CHECK(vmaCreateImage(m_ResourceCtx.m_Allocator, &imgInfo, &allocInfo, &img.image, &img.allocation, nullptr));

    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format == VK_FORMAT_D32_SFLOAT)
        aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

    VkImageViewCreateInfo fullView = VKInit::imageViewCreateInfo(img.image, format, aspect);

    fullView.subresourceRange.baseMipLevel = 0;
    fullView.subresourceRange.levelCount = img.mipLevels;

    fullView.subresourceRange.baseArrayLayer = 0;
    fullView.subresourceRange.layerCount = layers;

    if (isCubeMap)
        fullView.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

    VK_CHECK(vkCreateImageView(m_StateCtx.m_Device, &fullView, nullptr, &img.imageView));

    if (usage & VK_IMAGE_USAGE_STORAGE_BIT) {
        img.mipViews.resize(img.mipLevels);
        img.writeIdx.resize(img.mipLevels);

        for (uint32_t mip = 0; mip < img.mipLevels; mip++) {
            VkImageViewCreateInfo mipView =
                    VKInit::imageViewCreateInfo(img.image, format, aspect);

            mipView.subresourceRange.baseMipLevel = mip;
            mipView.subresourceRange.levelCount = 1;

            mipView.subresourceRange.baseArrayLayer = 0;
            mipView.subresourceRange.layerCount = layers;

            if (isCubeMap)
                mipView.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

            VK_CHECK(vkCreateImageView(m_StateCtx.m_Device, &mipView, nullptr, &img.mipViews[mip]));
        }
    }
    if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        img.readIdx = m_ResourceCtx.m_TextureAllocator.allocate();

        //todo store samplers seperately
        DescriptorWriter writer;
        if (isCubeMap) {
            writer.writeImage(
            0,
            img.imageView,
            getSampler(VKHelpers::getDefaultSamplerInfo()).sampler,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            img.readIdx);
        } else {
            writer.writeImage(
            0,
            img.imageView,
            getSampler(VKHelpers::getCubeMapSamplerInfo()).sampler,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            img.readIdx);
        }


        writer.updateSet(m_StateCtx.m_Device, m_ResourceCtx.m_TextureDescriptorSet);
    }
    if (usage & VK_IMAGE_USAGE_STORAGE_BIT) {
        for (uint32_t mip = 0; mip < img.mipLevels; mip++) {
            uint32_t index = m_ResourceCtx.m_StorageImageAllocator.allocate();
            img.writeIdx[mip] = index;

            DescriptorWriter writer;
            writer.writeImage(1, img.mipViews[mip], VK_NULL_HANDLE,
                              VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, index);

            writer.updateSet(m_StateCtx.m_Device, m_ResourceCtx.m_TextureDescriptorSet);
        }
    }

    spdlog::debug("Renderer: Created Image with {} mips", img.mipLevels);

    return img;
}
StoredSampler VKRenderer::getSampler(const VkSamplerCreateInfo& info) {
    if (m_ResourceCtx.m_SamplerMap.contains(info)) {
        return m_ResourceCtx.m_SamplerMap[info];
    }
    VkSampler sampler{};
    VK_CHECK(vkCreateSampler(m_StateCtx.m_Device, &info, nullptr, &sampler));
    m_ResourceCtx.m_DeletionQueue.pushFunction([=]() {
        vkDestroySampler(m_StateCtx.m_Device, sampler, nullptr);
    });
    uint32_t idx = m_ResourceCtx.m_SamplerAllocator.allocate();
    m_ResourceCtx.m_SamplerMap[info] = {sampler, idx};
    DescriptorWriter writer;
    writer.writeSampler(2, m_ResourceCtx.m_ErrorTexture.image.imageView, sampler, idx); //dummy image view
    writer.updateSet(m_StateCtx.m_Device, m_ResourceCtx.m_TextureDescriptorSet);
    return {sampler, idx};
}
void VKRenderer::generateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent3D size,
    uint32_t mipLevels, uint32_t layerCount) {
    if (mipLevels == 1) return;

    int32_t mipWidth  = static_cast<int32_t>(size.width);
    int32_t mipHeight = static_cast<int32_t>(size.height);

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;

        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.subresourceRange.levelCount = 1;

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};

        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = layerCount;

        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {
            std::max(1, mipWidth / 2),
            std::max(1, mipHeight / 2),
            1
        };

        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = layerCount;

        vkCmdBlitImage(
            cmd,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        mipWidth  = std::max(1, mipWidth / 2);
        mipHeight = std::max(1, mipHeight / 2);
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

AllocatedImage VKRenderer::createImage(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
                                       bool mipmapped, int layers) {
    size_t pixelSize = formatSize(format);
    size_t dataSize = size.width * size.height * pixelSize;
    AllocatedBuffer uploadBuffer = createBuffer(layers * dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                VMA_MEMORY_USAGE_CPU_TO_GPU);

    memcpy(uploadBuffer.allocationInfo.pMappedData, data, layers * dataSize);

    AllocatedImage newImage = createImage(size, format,
                                          usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                          mipmapped, layers);

    immediateSubmit([&](VkCommandBuffer cmd) {
        VKImages::transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        std::vector<VkBufferImageCopy> copyRegions(layers);
        for (int i = 0; i < layers; i++) {
            copyRegions[i].bufferOffset = i * dataSize;
            copyRegions[i].bufferRowLength = 0;
            copyRegions[i].bufferImageHeight = 0;

            copyRegions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegions[i].imageSubresource.mipLevel = 0;
            copyRegions[i].imageSubresource.baseArrayLayer = i;
            copyRegions[i].imageSubresource.layerCount = 1;

            copyRegions[i].imageOffset = {0, 0, 0};
            copyRegions[i].imageExtent = size;
        }

        vkCmdCopyBufferToImage(cmd, uploadBuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               copyRegions.size(),
                               copyRegions.data());
        if (newImage.mipLevels > 1) {
            generateMipmaps(cmd, newImage.image, size, newImage.mipLevels, layers);
        }
        VKImages::transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        spdlog::debug("Renderer: Uploaded Image");
    });

    destroyBuffer(uploadBuffer);
    return newImage;
}

void VKRenderer::destroyImage(const AllocatedImage &img) {
    vkDestroyImageView(m_StateCtx.m_Device, img.imageView, nullptr);
    for (auto &imgView : img.mipViews) {
        vkDestroyImageView(m_StateCtx.m_Device, imgView, nullptr);
    }
    vmaDestroyImage(m_ResourceCtx.m_Allocator, img.image, img.allocation);
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

    VK_CHECK(vmaCreateBuffer(m_ResourceCtx.m_Allocator, &bufferInfo, &allocInfo, &buffer.buffer,
        &buffer.allocation, &buffer.allocationInfo));

    //spdlog::debug("Renderer: Buffer Created");

    return buffer;
}

void VKRenderer::destroyBuffer(const AllocatedBuffer &buffer) {
    vmaDestroyBuffer(m_ResourceCtx.m_Allocator, buffer.buffer, buffer.allocation);
    //spdlog::debug("Renderer: Buffer Destroyed");
}
