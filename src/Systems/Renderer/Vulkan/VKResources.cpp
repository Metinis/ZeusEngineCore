#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>

#include "imgui_impl_vulkan.h"
#include "SkyboxRenderer.h"
#include "VkHelpers.h"
#include "VKInit.h"
#include "ZeusEngineCore/core/Application.h"
#include "ZeusEngineCore/core/Project.h"
#include "ZeusEngineCore/engine/CameraSystem.h"
#include "ZeusEngineCore/engine/Components.h"
#include "ZeusEngineCore/engine/Scene.h"
#include "ZeusEngineCore/engine/rendering/VKBackend.h"
#include "ZeusEngineCore/engine/rendering/VKContext.h"
#include "ZeusEngineCore/engine/rendering/VKUtils.h"

using namespace ZEN;

uint32_t IndexAllocator::allocate() {
    if (!availableList.empty()) {
        uint32_t idx = availableList.back();
        availableList.pop_back();
        return idx;
    }
    spdlog::error("Renderer: No available slot for texture!");
    return 0;
}

uint32_t IndexAllocator::allocate(uint32_t idx) {
    auto it = std::find(availableList.begin(), availableList.end(), idx);
    if (it != availableList.end()) {
        availableList.erase(it);
        return idx;
    }

    spdlog::error("Renderer: Requested index {} not available!", idx);
    return 0;
}

void IndexAllocator::free(uint32_t idx) {
    freeList.push_back(idx);
}

void IndexAllocator::init(uint32_t max) {
    availableList.resize(max);
    for (uint32_t i = 0; i < max; i++) {
        availableList[i] = max - 1 - i;
    }
}

void IndexAllocator::flush() {
    availableList.insert(availableList.end(), freeList.begin(), freeList.end());
    freeList.clear();
}

VKResources::VKResources() {
}

void VKResources::init(VKContext *stateCtx, RenderContext *renderCtx) {
    m_StateCtx = stateCtx;
    m_RenderCtx = renderCtx;

    initDescriptors();
    initMainSamplers();
    initPipelines();
    initErrorTexture();
}

void VKResources::initDescriptors() {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_StateCtx->m_PhysicalDevice, &props);
    VkPhysicalDeviceDescriptorIndexingPropertiesEXT indexingProps{};
    indexingProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT;

    VkPhysicalDeviceProperties2 props2{};
    props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    props2.pNext = &indexingProps;
    vkGetPhysicalDeviceProperties2(m_StateCtx->m_PhysicalDevice, &props2);
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, (float)props.limits.maxDescriptorSetStorageImages},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (float)props.limits.maxDescriptorSetSampledImages},
        {VK_DESCRIPTOR_TYPE_SAMPLER, (float)indexingProps.maxPerStageDescriptorUpdateAfterBindSamplers},
    };
    m_GlobalDescriptorAllocator.initPool(m_StateCtx->m_Device, 10, sizes);
    {
        DescriptorLayoutBuilder builder;
        builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        m_DrawImageDescriptorLayout = builder.build(m_StateCtx->m_Device, VK_SHADER_STAGE_COMPUTE_BIT);
    }
    {
        DescriptorLayoutBuilder builder;
        builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        builder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        m_FrameDescriptorLayout = builder.build(m_StateCtx->m_Device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);

        m_DeletionQueue.pushFunction([=]() {
            vkDestroyDescriptorSetLayout(m_StateCtx->m_Device, m_FrameDescriptorLayout, nullptr);
        });
    }
    {
        DescriptorLayoutBuilder builder;
        m_TextureAllocator.init(props.limits.maxDescriptorSetSampledImages);
        m_StorageImageAllocator.init(props.limits.maxDescriptorSetStorageImages);
        m_SamplerAllocator.init(indexingProps.maxPerStageDescriptorUpdateAfterBindSamplers);

        builder.addBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, props.limits.maxDescriptorSetSampledImages,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
        builder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, props.limits.maxDescriptorSetStorageImages,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
        builder.addBinding(2, VK_DESCRIPTOR_TYPE_SAMPLER, indexingProps.maxPerStageDescriptorUpdateAfterBindSamplers,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
        m_TextureDescriptorSetLayout = builder.build(m_StateCtx->m_Device, VK_SHADER_STAGE_VERTEX_BIT |
            VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
            nullptr, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);

        m_DeletionQueue.pushFunction([=]() {
            vkDestroyDescriptorSetLayout(m_StateCtx->m_Device, m_TextureDescriptorSetLayout, nullptr);
        });
    }
    {
        DescriptorLayoutBuilder builder;
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(m_StateCtx->m_PhysicalDevice, &props);

        m_MaterialAllocator.init(1000);

        builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        m_MaterialDescriptorSetLayout = builder.build(m_StateCtx->m_Device, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_DeletionQueue.pushFunction([=]() {
            vkDestroyDescriptorSetLayout(m_StateCtx->m_Device, m_MaterialDescriptorSetLayout, nullptr);
        });
    }
    m_DrawImageDescriptors = m_GlobalDescriptorAllocator.allocate(m_StateCtx->m_Device, m_DrawImageDescriptorLayout);
    m_MaterialDescriptorSet = m_GlobalDescriptorAllocator.allocate(m_StateCtx->m_Device, m_MaterialDescriptorSetLayout);
    m_TextureDescriptorSet = m_GlobalDescriptorAllocator.allocate(m_StateCtx->m_Device, m_TextureDescriptorSetLayout);
    {
        DescriptorWriter writer;
        writer.writeImage(0, m_RenderCtx->m_DrawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        writer.updateSet(m_StateCtx->m_Device, m_DrawImageDescriptors);
    }
    {
        DescriptorWriter writer;

        m_MaterialBuffer = createBuffer(sizeof(GPUMaterial) * 1000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU);

        writer.writeBuffer(0, m_MaterialBuffer.buffer, sizeof(GPUMaterial) * 1000, 0,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        writer.updateSet(m_StateCtx->m_Device, m_MaterialDescriptorSet);

        m_DeletionQueue.pushFunction([=]() {
            destroyBuffer(m_MaterialBuffer);
        });
    }

    m_DeletionQueue.pushFunction([=]() {
        m_TextureDescriptorAllocator.destroyPool(m_StateCtx->m_Device);
        m_GlobalDescriptorAllocator.destroyPool(m_StateCtx->m_Device);
        vkDestroyDescriptorSetLayout(m_StateCtx->m_Device, m_DrawImageDescriptorLayout, nullptr);
    });

    for (int i{}; i < FRAME_OVERLAP; ++i) {
        std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frameSizes {
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
        };
        m_RenderCtx->m_Frames[i].m_FrameDescriptors = DescriptorAllocatorGrowable{};
        m_RenderCtx->m_Frames[i].m_FrameDescriptors.init(m_StateCtx->m_Device, 1000, frameSizes);
        m_RenderCtx->m_Frames[i].m_SceneBuffer = createBuffer(sizeof(GPUSceneData),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        m_RenderCtx->m_Frames[i].m_ObjectBuffer = createBuffer(sizeof(GPUObjectData) * 10000,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        m_RenderCtx->m_Frames[i].m_IndirectBuffer = createBuffer(
        sizeof(VkDrawIndexedIndirectCommand) * 10000,
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
        );
        m_DeletionQueue.pushFunction([=]() {
            destroyBuffer(m_RenderCtx->m_Frames[i].m_IndirectBuffer);
            destroyBuffer(m_RenderCtx->m_Frames[i].m_ObjectBuffer);
            destroyBuffer(m_RenderCtx->m_Frames[i].m_SceneBuffer);
            m_RenderCtx->m_Frames[i].m_FrameDescriptors.destroyPools(m_StateCtx->m_Device);
        });
    }
    spdlog::debug("Renderer: Initialized Descriptors");
}

void VKResources::prepareDescriptors(VkCommandBuffer cmd) {
    //write to scene data buffer
    Scene* scene = Application::get().getCtx()->scene;
    CameraSystem* cameraSystem = Application::get().getCtx()->cameraSystem;
    auto lightDir = scene->getLightDir();
    glm::vec4 light = glm::vec4(lightDir.x, lightDir.y, lightDir.z, 1);
    auto cameraPos = scene->getCamera().getComponent<TransformComp>().getWorldPosition();

    auto* sceneUniformData = (GPUSceneData*)m_RenderCtx->getCurrentFrame().m_SceneBuffer.allocationInfo.pMappedData;
    m_SceneData = {};
    m_SceneData.view = cameraSystem->getView();
    m_SceneData.proj = cameraSystem->getProjection();
    m_SceneData.ambientColor = {0.5f, 0.5f, 0.5f, 1.0f};
    m_SceneData.sunlightColor = {1.0f, 1.0f, 1.0f, 1.0f};
    m_SceneData.sunlightDirection = light;
    m_SceneData.cameraPosition = glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1);

    *sceneUniformData = m_SceneData;

    VkDescriptorSet globalDescriptor = m_RenderCtx->getCurrentFrame().m_FrameDescriptors.
    allocate(m_StateCtx->m_Device, m_FrameDescriptorLayout);

    DescriptorWriter writer;
    writer.writeBuffer(0, m_RenderCtx->getCurrentFrame().m_SceneBuffer.buffer,
        sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    writer.writeBuffer(1, m_RenderCtx->getCurrentFrame().m_ObjectBuffer.buffer,
        sizeof(GPUObjectData) * 10000, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    writer.updateSet(m_StateCtx->m_Device, globalDescriptor);

    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
    m_MainPipelineLayout,0, 1, &globalDescriptor,0,nullptr);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_MainPipelineLayout,1, 1, &m_TextureDescriptorSet,0,nullptr);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_MainPipelineLayout,2, 1, &m_MaterialDescriptorSet,0,nullptr);

    GPUMainPushConstants pc {
        .skyboxIdx = m_RenderCtx->m_SkyboxRenderer->getSkyboxReadIdx(),
        .irradianceIdx = m_RenderCtx->m_SkyboxRenderer->getIrradianceReadIdx(),
        .prefilterMapIdx = m_RenderCtx->m_SkyboxRenderer->getPrefilterReadIdx(),
        .brdfTexIdx = m_RenderCtx->m_SkyboxRenderer->getBRDFReadIdx(),
    };
    vkCmdPushConstants(cmd, m_MainPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUMainPushConstants), &pc);

}
void VKResources::initPipelines() {
    initMainPipeLayout();
    initMainComputeLayout();
}

void VKResources::initErrorTexture() {
    //checkerboard image
    uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
    uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
    std::array<uint32_t, 16 *16 > pixels; //for 16x16 checkerboard texture
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            pixels[y*16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
        }
    }

    m_ErrorTexture = GPUTexture {
        .image = createImage(pixels.data(), VkExtent3D{16, 16, 1}, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT),
        .samplerIdx = getSampler(VKHelpers::getDefaultSamplerInfo()).idx,
    };
    DescriptorWriter writer;
    writer.updateSet(m_StateCtx->m_Device, m_TextureDescriptorSet);

    m_DeletionQueue.pushFunction([=](){
        destroyImage(m_ErrorTexture.image);
    });
}

void VKResources::initMainSamplers() {
    std::array<VkSamplerCreateInfo, 4> samplerInfos{
        VKHelpers::linearClampedSampler(),
        VKHelpers::nearestClampedSampler(),
        VKHelpers::linearRepeatSampler(),
        VKHelpers::nearestRepeatSampler(),
    };
    for (uint32_t i{}; i < samplerInfos.size(); ++i) {
        VkSampler sampler{};
        vkCreateSampler(m_StateCtx->m_Device, &samplerInfos[i], nullptr, &sampler);
        m_SamplerAllocator.allocate(i);
        DescriptorWriter writer;
        writer.writeSampler(2, m_ErrorTexture.image.imageView, sampler, i); //dummy image view
        writer.updateSet(m_StateCtx->m_Device, m_TextureDescriptorSet);
        m_SamplerMap[samplerInfos[i]] = {sampler, i};
        m_DeletionQueue.pushFunction([=]() {
            vkDestroySampler(m_StateCtx->m_Device, sampler, nullptr);
        });
    }
}

void VKResources::initMainPipeLayout() {
    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUMainPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo layoutInfo = VKInit::pipelineLayoutCreateInfo();
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;

    //todo maybe in the future make this more flexible
    std::array<VkDescriptorSetLayout, 3> setLayouts = {
        m_FrameDescriptorLayout,
        m_TextureDescriptorSetLayout,
        m_MaterialDescriptorSetLayout
    };

    layoutInfo.pSetLayouts = setLayouts.data();
    layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());

    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &bufferRange;

    VK_CHECK(vkCreatePipelineLayout(m_StateCtx->m_Device, &layoutInfo, nullptr, &m_MainPipelineLayout));

    m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipelineLayout(m_StateCtx->m_Device, m_MainPipelineLayout, nullptr);
    });
}

void VKResources::initMainComputeLayout() {
    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUComputePushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo layoutInfo = VKInit::pipelineLayoutCreateInfo();
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;

    //todo maybe in the future make this more flexible
    //frame descriptor unused
    std::array<VkDescriptorSetLayout, 2> setLayouts = {
        m_FrameDescriptorLayout,
        m_TextureDescriptorSetLayout,
    };

    layoutInfo.pSetLayouts = setLayouts.data();
    layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());

    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &bufferRange;

    VK_CHECK(vkCreatePipelineLayout(m_StateCtx->m_Device, &layoutInfo, nullptr, &m_ComputePipelineLayout));

    m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipelineLayout(m_StateCtx->m_Device, m_ComputePipelineLayout, nullptr);
    });
}

void VKResources::cleanup() {
    for (auto &buf: m_MeshMap | std::views::values) {
        destroyBuffer(buf.indexBuffer);
        destroyBuffer(buf.vertexBuffer);
    }
    for (auto &tex : m_TextureMap | std::views::values) {
        destroyImage(tex.texture.image);
    }
    m_DeletionQueue.flush();
}

VkPipeline VKResources::createMainPipeline(const PipelineInfo& pipelineInfo) {
    //todo check pipeline type, compute vs graphics, call this create in a getfunction
    //do lazy load
    VkShaderModule triangleFragShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        pipelineInfo.fragmentShader, m_StateCtx->m_Device, &triangleFragShader)) {
        std::cout << "Failed to load triangle frag shader" << std::endl;
        }
    VkShaderModule triangleVertShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        pipelineInfo.vertexShader, m_StateCtx->m_Device, &triangleVertShader)) {
        std::cout << "Failed to load triangle vert shader" << std::endl;
        }

    VKPipelineBuilder builder;
    builder.pipelineLayout = m_MainPipelineLayout;
    builder.setShaders(triangleVertShader, triangleFragShader);
    builder.setInputTopology(pipelineInfo.topology);
    builder.setPolygonMode(pipelineInfo.polygonMode);
    builder.setCullMode(pipelineInfo.cullMode, pipelineInfo.frontFace);
    if (!pipelineInfo.multisamplingEnabled)
        builder.setMultiSamplingNone();

    if (!pipelineInfo.depthTestEnabled) {
        builder.disableDepthTest();
    } else {
        builder.enableDepthTest(true, pipelineInfo.depthCompareOp);
        //todo allow different formats
        builder.setDepthFormat(m_RenderCtx->m_DepthImage.imageFormat);
    }

    if (pipelineInfo.blendingEnabled)
        builder.enableBlendingAdditive();
    else
        builder.disableBlending();

    builder.setColorAttachmentFormat(m_RenderCtx->m_DrawImage.imageFormat);


    VkPipeline pipeline = builder.buildPipeline(m_StateCtx->m_Device);

    vkDestroyShaderModule(m_StateCtx->m_Device, triangleFragShader, nullptr);
    vkDestroyShaderModule(m_StateCtx->m_Device, triangleVertShader, nullptr);

    m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipeline(m_StateCtx->m_Device, pipeline, nullptr);
    });
    spdlog::debug("Renderer: Initialized new main pipeline");

    m_PipelineMap[pipelineInfo] = pipeline;

    return pipeline;
}
GPUMeshBuffers VKResources::uploadMesh(AssetID id, const MeshData &mesh) {
    if (m_MeshMap.contains(id)) {
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
    newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_StateCtx->m_Device, &deviceAddressInfo);

    newSurface.indexBuffer = createBuffer(indexBufferSize,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                                          | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                          VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = createBuffer(vertexBufferSize + indexBufferSize,
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void *data = staging.allocationInfo.pMappedData;

    memcpy(data, mesh.vertices.data(), vertexBufferSize);
    memcpy((char *) data + vertexBufferSize, mesh.indices.data(), indexBufferSize);

    m_RenderCtx->immediateSubmit([&](VkCommandBuffer cmd) {
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

GPUTexture VKResources::uploadTexture(AssetID id, const TextureData &texture) {
    LoadedTexture texturePixels = loadPixelData(texture);
    AllocatedImage newTexture = createImage((void *) texturePixels.pixels.data(),
                                            VkExtent3D{
                                                (unsigned int) texturePixels.texWidth,
                                                (unsigned int) texturePixels.texHeight, 1
                                            },
                                            texture.format, VK_IMAGE_USAGE_SAMPLED_BIT, false, texturePixels.layers);

    StoredSampler sampler = getSampler(VKHelpers::toVkSamplerCreateInfo(texture.samplerInfo));
    //--------------------------------------------------------
    if (m_TextureMap.contains(id)) {
        destroyImage(m_TextureMap[id].texture.image);
        m_TextureAllocator.free(m_TextureMap[id].idx);
    }

    GPUTexture gpuTex = {
        .image = newTexture,
        .samplerIdx = sampler.idx,
    };

    m_TextureMap[id] = {gpuTex, newTexture.readIdx, texture.type};
    spdlog::debug("Renderer: Created Texture ID: {}", (uint64_t) id);
    return gpuTex;
}

GPUMaterial VKResources::uploadMaterial(const AssetID id, const Material &material) {
    GPUMaterial gpuMat = {
        .u_Albedo = glm::vec4(material.albedo.x, material.albedo.y, material.albedo.z, 1.0f),
        .u_Params = glm::vec4(material.metallic, material.roughness, material.ao, 1.0),
    };

    if (m_TextureMap.contains(material.texture)) {
        gpuMat.albedoIndex = m_TextureMap[material.texture].idx;
    }
    if (m_TextureMap.contains(material.metallicTex)) {
        gpuMat.metallicIndex = m_TextureMap[material.metallicTex].idx;
    }
    if (m_TextureMap.contains(material.roughnessTex)) {
        gpuMat.roughnessIndex = m_TextureMap[material.roughnessTex].idx;
    }
    if (m_TextureMap.contains(material.normalTex)) {
        gpuMat.normalIndex = m_TextureMap[material.normalTex].idx;
    }
    if (m_TextureMap.contains(material.aoTex)) {
        gpuMat.aoIndex = m_TextureMap[material.aoTex].idx;
    }

    //todo do this appropriately
    gpuMat.samplerIndex = m_TextureMap[material.texture].texture.samplerIdx;

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
    if (m_PipelineMap.contains(material.pipelineInfo)) {
        pipeline = m_PipelineMap[material.pipelineInfo];
    } else {
        pipeline = createMainPipeline(material.pipelineInfo);
    }
    bool useDepth = material.pipelineInfo.depthTestEnabled;

    DescriptorWriter writer;

    uint32_t idx{};

    //create new, otherwise replace
    if (!m_MaterialMap.contains(id)) {
        idx = m_MaterialAllocator.allocate();
    } else {
        idx = m_MaterialMap[id].idx;
    }

    m_MaterialMap[id] = {gpuMat, pipeline, useDepth, idx};

    auto *mapped = (GPUMaterial *) m_MaterialBuffer.allocationInfo.pMappedData;
    mapped[idx] = gpuMat;
    spdlog::debug("Renderer: Created Material ID: {}", (uint64_t) id);
    return gpuMat;
}

void VKResources::deleteMaterial(AssetID id) {
    if (m_MaterialMap.contains(id)) {
        auto material = m_MaterialMap[id];
        uint32_t frameIndex = (m_RenderCtx->m_FrameNumber + FRAME_OVERLAP) % FRAME_OVERLAP;
        m_RenderCtx->m_Frames[frameIndex].m_DeletionQueue.pushFunction([=]() {
            m_MaterialAllocator.free(material.idx);
        });
        m_MaterialMap.erase(id);

        spdlog::debug("Renderer: Deleted Material ID: {}", (uint64_t) id);
        return;
    }
    spdlog::error("Renderer: Attempt to delete non-existing Material! ID: {}", (uint64_t) id);
}

void VKResources::deleteMesh(AssetID id) {
    if (m_MeshMap.contains(id)) {
        auto &meshBuf = m_MeshMap[id];
        uint32_t frameIndex = (m_RenderCtx->m_FrameNumber + FRAME_OVERLAP) % FRAME_OVERLAP;
        m_RenderCtx->m_Frames[frameIndex].m_DeletionQueue.pushFunction([=]() {
            destroyBuffer(meshBuf.indexBuffer);
            destroyBuffer(meshBuf.vertexBuffer);
        });
        m_MeshMap.erase(id);
        spdlog::debug("Deleted GPU Mesh ID: {}", (uint64_t) id);
        return;
    }
    spdlog::error("Attempt to delete non-existing GPU Mesh! ID: {}", (uint64_t) id);
}

void VKResources::deleteTexture(AssetID id) {
    if (m_TextureMap.contains(id)) {
        auto tex = m_TextureMap[id];
        uint32_t frameIndex = (m_RenderCtx->m_FrameNumber + FRAME_OVERLAP) % FRAME_OVERLAP;
        m_RenderCtx->m_Frames[frameIndex].m_DeletionQueue.pushFunction([=]() {
            if (m_ImGUIDescSetMap.contains(id)) {
                ImGui_ImplVulkan_RemoveTexture(m_ImGUIDescSetMap[id]);
                m_ImGUIDescSetMap.erase(id);
            }
            m_TextureAllocator.free(tex.idx);
            destroyImage(tex.texture.image);
        });
        m_TextureMap.erase(id);
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

AllocatedImage VKResources::createImage(VkExtent3D size, VkFormat format,
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

    VK_CHECK(vmaCreateImage(m_Allocator, &imgInfo, &allocInfo, &img.image, &img.allocation, nullptr));

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

    VK_CHECK(vkCreateImageView(m_StateCtx->m_Device, &fullView, nullptr, &img.imageView));

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

            VK_CHECK(vkCreateImageView(m_StateCtx->m_Device, &mipView, nullptr, &img.mipViews[mip]));
        }
    }
    if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        img.readIdx = m_TextureAllocator.allocate();

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


        writer.updateSet(m_StateCtx->m_Device, m_TextureDescriptorSet);
    }
    if (usage & VK_IMAGE_USAGE_STORAGE_BIT) {
        for (uint32_t mip = 0; mip < img.mipLevels; mip++) {
            uint32_t index = m_StorageImageAllocator.allocate();
            img.writeIdx[mip] = index;

            DescriptorWriter writer;
            writer.writeImage(1, img.mipViews[mip], VK_NULL_HANDLE,
                              VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, index);

            writer.updateSet(m_StateCtx->m_Device, m_TextureDescriptorSet);
        }
    }

    spdlog::debug("Renderer: Created Image with {} mips", img.mipLevels);

    return img;
}
StoredSampler VKResources::getSampler(const VkSamplerCreateInfo& info) {
    if (m_SamplerMap.contains(info)) {
        return m_SamplerMap[info];
    }
    VkSampler sampler{};
    VK_CHECK(vkCreateSampler(m_StateCtx->m_Device, &info, nullptr, &sampler));
    m_DeletionQueue.pushFunction([=]() {
        vkDestroySampler(m_StateCtx->m_Device, sampler, nullptr);
    });
    uint32_t idx = m_SamplerAllocator.allocate();
    m_SamplerMap[info] = {sampler, idx};
    DescriptorWriter writer;
    writer.writeSampler(2, m_ErrorTexture.image.imageView, sampler, idx); //dummy image view
    writer.updateSet(m_StateCtx->m_Device, m_TextureDescriptorSet);
    return {sampler, idx};
}
void VKResources::generateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent3D size,
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

AllocatedImage VKResources::createImage(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
                                       bool mipmapped, int layers) {
    size_t pixelSize = formatSize(format);
    size_t dataSize = size.width * size.height * pixelSize;
    AllocatedBuffer uploadBuffer = createBuffer(layers * dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                VMA_MEMORY_USAGE_CPU_TO_GPU);

    memcpy(uploadBuffer.allocationInfo.pMappedData, data, layers * dataSize);

    AllocatedImage newImage = createImage(size, format,
                                          usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                          mipmapped, layers);

    m_RenderCtx->immediateSubmit([&](VkCommandBuffer cmd) {
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

void VKResources::destroyImage(const AllocatedImage &img) {
    vkDestroyImageView(m_StateCtx->m_Device, img.imageView, nullptr);
    for (auto &imgView : img.mipViews) {
        vkDestroyImageView(m_StateCtx->m_Device, imgView, nullptr);
    }
    vmaDestroyImage(m_Allocator, img.image, img.allocation);
    spdlog::debug("Renderer: Deleted Image");
}

AllocatedBuffer VKResources::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
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

void VKResources::destroyBuffer(const AllocatedBuffer &buffer) {
    vmaDestroyBuffer(m_Allocator, buffer.buffer, buffer.allocation);
    //spdlog::debug("Renderer: Buffer Destroyed");
}

void VKResources::initAllocator(VKContext* stateCtx) {
    VmaAllocatorCreateInfo allocatorCreateInfo {};
    allocatorCreateInfo.device = stateCtx->m_Device;
    allocatorCreateInfo.instance = stateCtx->m_Instance;
    allocatorCreateInfo.physicalDevice = stateCtx->m_PhysicalDevice;
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator);
    m_DeletionQueue.pushFunction([=]() {
        vmaDestroyAllocator(m_Allocator);
    });
}

ImGui_ImplVulkan_InitInfo VKResources::initImgui() {
    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(m_StateCtx->m_Device, &poolInfo, nullptr, &imguiPool));

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_StateCtx->m_Instance;
    initInfo.PhysicalDevice = m_StateCtx->m_PhysicalDevice;
    initInfo.Device = m_StateCtx->m_Device;
    initInfo.Queue = m_StateCtx->m_GraphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;

    //dynamic rendering parameters stuff
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_RenderCtx->m_SwapChainImageFormat;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);
    m_ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(getSampler(VKHelpers::getDefaultSamplerInfo()).sampler, m_RenderCtx->m_DrawImage.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    m_ImGUIErrorSet = ImGui_ImplVulkan_AddTexture(getSampler(VKHelpers::getDefaultSamplerInfo()).sampler, m_ErrorTexture.image.imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );


    m_DeletionQueue.pushFunction([=]() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(m_StateCtx->m_Device, imguiPool, nullptr);

    });
    spdlog::debug("Renderer: ImGUI Initialized");

    return initInfo;
}

VkDescriptorSet VKResources::getImGUIDescSet(AssetID id) {
    if (m_ImGUIDescSetMap.contains(id)) {
        return m_ImGUIDescSetMap[id];
    }
    //if not found, add this id for cache
    //todo check the sketchy NULL HANDLE
    if (m_TextureMap.contains(id) && m_TextureMap[id].texture.image.imageView != VK_NULL_HANDLE) {
        spdlog::debug("Renderer: Cached Thumbnail Tex: {}", (uint64_t)id);
        auto& tex = m_TextureMap[id];
        if (tex.type == TextureType::Texture2D || tex.type == Texture2DAssimp) {
            m_ImGUIDescSetMap.insert({id, ImGui_ImplVulkan_AddTexture(getSampler(
                VKHelpers::getDefaultSamplerInfo()).sampler, tex.texture.image.imageView,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)});
        } else {
            m_ImGUIDescSetMap.insert({id, ImGui_ImplVulkan_AddTexture(getSampler(
                VKHelpers::getDefaultSamplerInfo()).sampler, m_ErrorTexture.image.imageView,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)});
        }

        return m_ImGUIDescSetMap[id];
    }
    return m_ImGUIErrorSet;
}
