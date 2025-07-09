#include "VulkanPipeline.h"
#include "../../../Utils.h"

constexpr auto viewportState_v =
        vk::PipelineViewportStateCreateInfo({}, 1, {}, 1);

constexpr auto dynamicStates_v = std::array{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
        vk::DynamicState::eLineWidth,
};

Pipeline::Pipeline(PipelineCreateInfo const& pipelineCreateInfo) : m_CreateInfo(pipelineCreateInfo){
    std::string vertexPath = "D:/Denio/Projektai/ProjectZeus/ZeusEditor/resources/shaders/vkbasic.vert.spv";
    std::string fragmentPath = "D:/Denio/Projektai/ProjectZeus/ZeusEditor/resources/shaders/vkbasic.frag.spv";
    const auto vertexSpirv = ToSpirV(vertexPath);
    const auto fragmentSpirv = ToSpirV(fragmentPath);

    if (vertexSpirv.empty() || fragmentSpirv.empty()) {
        throw std::runtime_error{"Failed to load shaders"};
    }

    m_PipelineLayout = pipelineCreateInfo.pipelineLayout;

    vk::ShaderModuleCreateInfo vertexCreateInfo{};
    vertexCreateInfo.setCode(vertexSpirv);

    vk::ShaderModuleCreateInfo fragmentCreateInfo{};
    fragmentCreateInfo.setCode(fragmentSpirv);

    auto const vertexShader = pipelineCreateInfo.device.createShaderModuleUnique(vertexCreateInfo);
    auto const fragmentShader = pipelineCreateInfo.device.createShaderModuleUnique(fragmentCreateInfo);
    auto const pipelineState = PipelineState{
            .vertexShader = *vertexShader,
            .fragmentShader = *fragmentShader,
    };
    Build(pipelineState);

}

[[nodiscard]] std::array<vk::PipelineShaderStageCreateInfo, 2> CreateShaderStages(vk::ShaderModule const vertex,
                                        vk::ShaderModule const fragment) {
    // set vertex (0) and fragment (1) shader stages.
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageCreateInfo{};
    shaderStageCreateInfo[0]
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setPName("main")
            .setModule(vertex);
    shaderStageCreateInfo[1]
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setPName("main")
            .setModule(fragment);
    return shaderStageCreateInfo;
}

[[nodiscard]] constexpr vk::PipelineDepthStencilStateCreateInfo CreateDepthStencilState(std::uint8_t flags,
                           vk::CompareOp const depth_compare) {
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
    auto const depthTest =
            (flags & PipelineFlag::DepthTest) == PipelineFlag::DepthTest;
    depthStencilStateCreateInfo.setDepthTestEnable(depthTest ? vk::True : vk::False)
            .setDepthCompareOp(depth_compare);
    return depthStencilStateCreateInfo;
}

[[nodiscard]] constexpr vk::PipelineColorBlendAttachmentState CreateColorBlendAttachment(std::uint8_t const flags) {
    vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
    auto const alphaBlend =
            (flags & PipelineFlag::AlphaBlend) == PipelineFlag::AlphaBlend;
    using CCF = vk::ColorComponentFlagBits;
    pipelineColorBlendAttachmentState.setColorWriteMask(CCF::eR | CCF::eG | CCF::eB | CCF::eA)
            .setBlendEnable(alphaBlend ? vk::True : vk::False)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
            .setAlphaBlendOp(vk::BlendOp::eAdd);
    return pipelineColorBlendAttachmentState;
}

void Pipeline::Build(PipelineState const& state){
    auto const shaderStages = CreateShaderStages(state.vertexShader, state.fragmentShader);
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.setVertexAttributeDescriptions(state.vertexAttributes);
    vertexInputCreateInfo.setVertexBindingDescriptions(state.vertexBindings);

    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.setRasterizationSamples(m_CreateInfo.samples);
    multisampleStateCreateInfo.setSampleShadingEnable(vk::False);

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{{}, state.topology};

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.setPolygonMode(state.polygonMode);
    rasterizationStateCreateInfo.setCullMode(state.cullMode);

    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo =
            CreateDepthStencilState(state.flags, state.depthCompare);

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = CreateColorBlendAttachment(state.flags);

    vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
    pipelineColorBlendStateCreateInfo.setAttachments(colorBlendAttachmentState);

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.setDynamicStates(dynamicStates_v);

    vk::PipelineRenderingCreateInfo renderingCreateInfo{};

    if(m_CreateInfo.colorFormat != vk::Format::eUndefined){
        renderingCreateInfo.setColorAttachmentFormats(m_CreateInfo.colorFormat);
    }
    renderingCreateInfo.setDepthAttachmentFormat(m_CreateInfo.depthFormat);

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.setLayout(m_PipelineLayout);
    pipelineCreateInfo.setStages(shaderStages);
    pipelineCreateInfo.setPVertexInputState(&vertexInputCreateInfo);
    pipelineCreateInfo.setPViewportState(&viewportState_v);
    pipelineCreateInfo.setPMultisampleState(&multisampleStateCreateInfo);
    pipelineCreateInfo.setPInputAssemblyState(&inputAssemblyStateCreateInfo);
    pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo);
    pipelineCreateInfo.setPDepthStencilState(&depthStencilStateCreateInfo);
    pipelineCreateInfo.setPColorBlendState(&pipelineColorBlendStateCreateInfo);
    pipelineCreateInfo.setPDynamicState(&dynamicStateCreateInfo);
    pipelineCreateInfo.setPNext(&renderingCreateInfo);

    vk::Pipeline pipeline;
    if(m_CreateInfo.device.createGraphicsPipelines({}, 1, &pipelineCreateInfo, {}, &pipeline) != vk::Result::eSuccess){
        std::printf("Failed to create graphics pipeline!");
    }
    m_Pipeline = std::move(vk::UniquePipeline{pipeline, m_CreateInfo.device});
}