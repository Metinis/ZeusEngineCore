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
    initComputePipeline();
    //skybox stuff
    TextureData data {
        .path = Application::get().getResourceRoot() + path.string(),
        .type = Texture2DHDR,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .dimensions = {1024, 1024},
    };
    m_UploadedImage = m_Renderer->uploadTexture(AssetID(), data);

    m_EqMap = m_Renderer->createImage(VkExtent3D {512, 512, 1}, VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, false, 6);
    m_Renderer->m_DeletionQueue.pushFunction([=] {
        m_Renderer->destroyImage(m_EqMap);
    });

    spdlog::debug("Skybox: Initialized Skybox Resources");
}
void SkyboxRenderer::initComputePipeline() {
    VkShaderModule computeDrawShader;
    if (!VKPipelines::loadShaderModule(Application::get().getResourceRoot() +
        "/shaders/vulkan-shaders/gradient.comp.spv", m_Renderer->m_Device, &computeDrawShader)) {
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
void SkyboxRenderer::render(VkCommandBuffer cmd) {
    if (m_IsDirty) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_EqMapPipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_Renderer->m_ComputePipelineLayout, 1,
            1, &m_Renderer->m_TextureDescriptorSet, 0, nullptr);
        GPUComputePushConstants pc {
            .eqTextureIdx = 1,
            .skyboxIdx = m_EqMap.writeIdx
        };
        vkCmdPushConstants(cmd, m_Renderer->m_ComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GPUComputePushConstants), &pc);

        vkCmdDispatch(cmd, std::ceil(m_EqMap.imageExtent.width / 16.0),
            std::ceil(m_EqMap.imageExtent.height / 16.0), 6);
        m_IsDirty = false;
        spdlog::debug("Skybox: Generated");
    }
}

void SkyboxRenderer::cleanup() {
    spdlog::debug("Skybox: Cleanup");
}

SkyboxRenderer::~SkyboxRenderer() {
    cleanup();
}
