#include "PipelineBuilder.h"
#include "ZeusEngineCore/Utils.h"

using namespace ZEN::VKAPI;

constexpr auto viewportState_v =
        vk::PipelineViewportStateCreateInfo({}, 1, {}, 1);

constexpr auto dynamicStates_v = std::array{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
        vk::DynamicState::eLineWidth,
        vk::DynamicState::ePolygonModeEXT //must have dynamic state 3 features enabled!!!
};

PipelineBuilder::PipelineBuilder(PipelineBuilderCreateInfo const& pipelineBuilderCreateInfo) : m_CreateInfo(pipelineBuilderCreateInfo){

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
    //auto const depthTest =
    //        (flags & PipelineFlag::DepthTest) == PipelineFlag::DepthTest;
    //depthStencilStateCreateInfo.setDepthTestEnable(depthTest ? vk::True : vk::False)
    //        .setDepthCompareOp(depth_compare);
    depthStencilStateCreateInfo.setDepthWriteEnable(vk::True);
    depthStencilStateCreateInfo.setDepthTestEnable(vk::True);
    depthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);
    depthStencilStateCreateInfo.setDepthBoundsTestEnable(vk::True);
    depthStencilStateCreateInfo.setMinDepthBounds(0.0f);
    depthStencilStateCreateInfo.setMaxDepthBounds(1.0f);
    depthStencilStateCreateInfo.setStencilTestEnable(vk::False);

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

vk::UniquePipeline PipelineBuilder::Build(vk::PipelineLayout const layout, PipelineState const& state){
    auto const shaderStages = CreateShaderStages(state.vertexShader, state.fragmentShader);
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.setVertexAttributeDescriptions(state.vertexAttributes);
    vertexInputCreateInfo.setVertexBindingDescriptions(state.vertexBindings);

    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.setRasterizationSamples(m_CreateInfo.samples);
    multisampleStateCreateInfo.setSampleShadingEnable(vk::False);

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{{}, state.topology};

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.setLineWidth(1.0f)
            .setDepthClampEnable(VK_FALSE)
            .setRasterizerDiscardEnable(VK_FALSE)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setCullMode(state.cullMode)
            .setPolygonMode(state.polygonMode);


    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo =
            CreateDepthStencilState(state.flags, state.depthCompare);

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = CreateColorBlendAttachment(state.flags);

    vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
    pipelineColorBlendStateCreateInfo.setAttachments(colorBlendAttachmentState);

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.setDynamicStates(dynamicStates_v);

    vk::PipelineRenderingCreateInfo renderingCreateInfo{};

    if(m_CreateInfo.colorFormat != vk::Format::eUndefined){
        renderingCreateInfo.setColorAttachmentFormats({m_CreateInfo.colorFormat});
    }
    renderingCreateInfo.setDepthAttachmentFormat(m_CreateInfo.depthFormat);

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.setLayout(layout);
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
    return vk::UniquePipeline{pipeline, m_CreateInfo.device};
}