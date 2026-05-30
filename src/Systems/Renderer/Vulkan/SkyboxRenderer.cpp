#include "SkyboxRenderer.h"

#include <vulkan/vk_enum_string_helper.h>

#include "VKInit.h"
#include "ZeusEngineCore/core/Application.h"
#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#include "ZeusEngineCore/engine/rendering/VKUtils.h"

using namespace ZEN;

SkyboxRenderer::SkyboxRenderer(VKRenderer* renderer) : m_Renderer(renderer) {

}

void SkyboxRenderer::init(std::filesystem::path const &path) {
    initEqMapPipeline();
    initIrradiancePipeline();
    initPrefilterPipeline();
    initBRDFPipeline();
    //skybox stuff
    TextureData data {
        .path = Application::get().getResourceRoot() + path.string(),
        .type = Texture2DHDR,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .dimensions = {512, 512},
    };
    m_UploadedImage = m_Renderer->uploadTexture(AssetID(), data);

    m_EqMap = m_Renderer->createImage(VkExtent3D {512, 512, 1}, VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, false, 6);

    m_IrradianceMap = m_Renderer->createImage(VkExtent3D {32, 32, 1}, VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, false, 6);

    m_PrefilterMap = m_Renderer->createImage(VkExtent3D {128, 128, 1}, VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, true, 6);

    m_BRDFTex = m_Renderer->createImage(VkExtent3D {512, 512, 1}, VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, false, 1);

    m_Renderer->m_DeletionQueue.pushFunction([=] {
        m_Renderer->destroyImage(m_EqMap);
        m_Renderer->destroyImage(m_IrradianceMap);
        m_Renderer->destroyImage(m_PrefilterMap);
        m_Renderer->destroyImage(m_BRDFTex);
    });

    spdlog::debug("Skybox: Initialized Skybox Resources");
}
void SkyboxRenderer::initEqMapPipeline() {
    VkShaderModule computeDrawShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        "/shaders/vulkan-shaders/eq-to-cubemap.comp.spv", m_Renderer->m_Device, &computeDrawShader)) {
        std::cout << "Failed to load compute draw shader" << std::endl;
        }
    VkPipelineShaderStageCreateInfo stageInfo = VKInit::pipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_COMPUTE_BIT, computeDrawShader);

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_Renderer->m_ComputePipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    VK_CHECK(vkCreateComputePipelines(m_Renderer->m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
        &m_EqMapPipeline));

    vkDestroyShaderModule(m_Renderer->m_Device, computeDrawShader, nullptr);
    m_Renderer->m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipeline(m_Renderer->m_Device, m_EqMapPipeline, nullptr);
    });
    spdlog::debug("Skybox: Eq Map Pipeline Initialized");
}

void SkyboxRenderer::initIrradiancePipeline() {
    VkShaderModule computeDrawShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        "/shaders/vulkan-shaders/irradiance.comp.spv", m_Renderer->m_Device, &computeDrawShader)) {
        std::cout << "Failed to load compute draw shader" << std::endl;
        }
    VkPipelineShaderStageCreateInfo stageInfo = VKInit::pipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_COMPUTE_BIT, computeDrawShader);

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_Renderer->m_ComputePipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    VK_CHECK(vkCreateComputePipelines(m_Renderer->m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
        &m_IrradianceMapPipeline));

    vkDestroyShaderModule(m_Renderer->m_Device, computeDrawShader, nullptr);
    m_Renderer->m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipeline(m_Renderer->m_Device, m_IrradianceMapPipeline, nullptr);
    });
    spdlog::debug("Skybox: Irradiance Pipeline Initialized");
}

void SkyboxRenderer::initPrefilterPipeline() {
    VkShaderModule computeDrawShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        "/shaders/vulkan-shaders/prefilter.comp.spv", m_Renderer->m_Device, &computeDrawShader)) {
        std::cout << "Failed to load compute draw shader" << std::endl;
        }
    VkPipelineShaderStageCreateInfo stageInfo = VKInit::pipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_COMPUTE_BIT, computeDrawShader);

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_Renderer->m_ComputePipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    VK_CHECK(vkCreateComputePipelines(m_Renderer->m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
        &m_PrefilterPipeline));

    vkDestroyShaderModule(m_Renderer->m_Device, computeDrawShader, nullptr);
    m_Renderer->m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipeline(m_Renderer->m_Device, m_PrefilterPipeline, nullptr);
    });
    spdlog::debug("Skybox: Prefilter Pipeline Initialized");
}

void SkyboxRenderer::initBRDFPipeline() {
    VkShaderModule computeDrawShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        "/shaders/vulkan-shaders/brdf.comp.spv", m_Renderer->m_Device, &computeDrawShader)) {
        std::cout << "Failed to load compute draw shader" << std::endl;
        }
    VkPipelineShaderStageCreateInfo stageInfo = VKInit::pipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_COMPUTE_BIT, computeDrawShader);

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_Renderer->m_ComputePipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    VK_CHECK(vkCreateComputePipelines(m_Renderer->m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
        &m_BRDFPipeline));

    vkDestroyShaderModule(m_Renderer->m_Device, computeDrawShader, nullptr);
    m_Renderer->m_DeletionQueue.pushFunction([=]() {
        vkDestroyPipeline(m_Renderer->m_Device, m_BRDFPipeline, nullptr);
    });
    spdlog::debug("Skybox: Prefilter Pipeline Initialized");
}

void SkyboxRenderer::render(VkCommandBuffer cmd) {
    if (m_IsDirty) {
        //EQ to cubemap
        {
            GPUComputePushConstants pc {
                .eqTextureIdx = m_UploadedImage.image.readIdx,
                .eqMapIdx = m_EqMap.writeIdx[0],
                .conMapIdx = m_IrradianceMap.readIdx,
                .prefilterMapIdx = m_PrefilterMap.readIdx
            };
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_EqMapPipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_Renderer->m_ComputePipelineLayout, 1,
                1, &m_Renderer->m_TextureDescriptorSet, 0, nullptr);
            vkCmdPushConstants(cmd, m_Renderer->m_ComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GPUComputePushConstants), &pc);

            vkCmdDispatch(cmd, std::ceil(m_EqMap.imageExtent.width / 16.0),
                std::ceil(m_EqMap.imageExtent.height / 16.0), 6);
            spdlog::debug("Skybox: Eq Map Generated");
        }
        //Irradiance Map
        {
            GPUComputePushConstants pc {
                .eqTextureIdx = m_UploadedImage.image.readIdx,
                .eqMapIdx = m_EqMap.readIdx,
                .conMapIdx = m_IrradianceMap.writeIdx[0],
                .prefilterMapIdx = m_PrefilterMap.readIdx
            };
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_IrradianceMapPipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_Renderer->m_ComputePipelineLayout, 1,
                1, &m_Renderer->m_TextureDescriptorSet, 0, nullptr);
            vkCmdPushConstants(cmd, m_Renderer->m_ComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GPUComputePushConstants), &pc);

            vkCmdDispatch(cmd, std::ceil(m_IrradianceMap.imageExtent.width / 16.0),
                std::ceil(m_IrradianceMap.imageExtent.height / 16.0), 6);
            spdlog::debug("Skybox: Irradiance Map Generated");
        }
        //Prefilter Map
        {
            GPUComputePushConstants pc {
                .eqTextureIdx = m_UploadedImage.image.readIdx,
                .eqMapIdx = m_EqMap.readIdx,
                .conMapIdx = m_IrradianceMap.readIdx,
                .prefilterMapIdx = m_PrefilterMap.writeIdx[0],
            };
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_PrefilterPipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_Renderer->m_ComputePipelineLayout, 1,
                1, &m_Renderer->m_TextureDescriptorSet, 0, nullptr);

            for (int mip = 0; mip < m_PrefilterMap.mipLevels; ++mip) {
                uint w = m_PrefilterMap.imageExtent.width >> mip;
                uint h = m_PrefilterMap.imageExtent.height >> mip;

                pc.prefilterMapIdx = m_PrefilterMap.writeIdx[mip];
                pc.prefilterBaseWidth  = w;
                pc.prefilterBaseHeight = h;
                float roughness = (float)mip / (float)(m_PrefilterMap.mipLevels - 1);
                pc.roughness = roughness;

                vkCmdPushConstants(cmd, m_Renderer->m_ComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                    0, sizeof(GPUComputePushConstants), &pc);

                vkCmdDispatch(cmd,(w + 15) / 16.0,(h + 15) / 16.0, 6);
            }
            spdlog::debug("Skybox: Prefilter Map Generated");
        }
        //BRDF texture
        {
            GPUComputePushConstants pc {
                .eqTextureIdx = m_UploadedImage.image.readIdx,
                .eqMapIdx = m_EqMap.readIdx,
                .conMapIdx = m_IrradianceMap.readIdx,
                .prefilterMapIdx = m_PrefilterMap.readIdx,
                .brdfTexIdx = m_BRDFTex.writeIdx[0],
            };
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_BRDFPipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_Renderer->m_ComputePipelineLayout, 1,
                1, &m_Renderer->m_TextureDescriptorSet, 0, nullptr);
            vkCmdPushConstants(cmd, m_Renderer->m_ComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GPUComputePushConstants), &pc);

            vkCmdDispatch(cmd, std::ceil(m_BRDFTex.imageExtent.width / 16.0),
                std::ceil(m_BRDFTex.imageExtent.height / 16.0), 6);
            spdlog::debug("Skybox: BRDF Texture Generated");
        }
        m_IsDirty = false;
    }
}

void SkyboxRenderer::cleanup() {
    spdlog::debug("Skybox: Cleanup");
}

SkyboxRenderer::~SkyboxRenderer() {
    cleanup();
}
